#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task_stack.h"

event_t 	event_list[MAX_EVENT_COUNT];
uint32_t 	event_count = 0;

//Inicializo la estructura de la lista de evento
void os_event_init_array(void){
	int i;
	for(i=0;i<MAX_EVENT_COUNT;i++){
		event_list[i].state = EVENT_INIT;
		//Las tareas que apunta las ponemos en el maximo posible + 1
		event_list[i].task_waiting = MAX_TASK_COUNT+1;
	}
}

os_event_handler_t 	os_event_init	(void){
	os_event_handler_t event_handler;
	//Si queremos crear un evento mas de lo que podemos directamente
	//devolvemos NULL
	if(event_count >= MAX_EVENT_COUNT ){
		return NULL;
	}
	//Los eventos arrancan esperando que alguien haga un kick
	event_list[event_count].state = EVENT_INIT;
	event_handler = &event_list[event_count];
	//Sumamos la cantidad de eventos que tenemos inicializados
	event_count++;
	//Devolvemos el handler al evento para que pueda ser utilizado.
	return event_handler;
}

bool_t os_event_wait	(os_event_handler_t event){
	//hago un casteo para poder obtener los datos que corresponden y poder tocarlos
	event_t*	ev = (event_t*)(event);
	switch(ev->state){
	case EVENT_INIT:
		if(running_task_index > task_count){
			//Esta es una condición imposible
			return FALSE;
		}
		//La que llama al wait por primera vez es la tarea que va a esperar.
		ev->state = EVENT_WAIT;
		ev->task_waiting = running_task_index;
		//Ponemos la tarea en modo de waiting

		//Habilitamos una sección donde el Systick nos puede cambiar el estado de las tareas
		disable_sys_tick_irq();
		task_list[running_task_index].event_waiting = TRUE;
		task_list[running_task_index].state 				= TASK_SLEEPING;
		enable_sys_tick_irq();
		break;
	case EVENT_WAIT:
		if((running_task_index > task_count) || (ev->task_waiting != running_task_index)){
			//Esta es una condición imposible ya que una vez que una tarea que hizo el primer wait
			//no puede cambiar. Es restrictivo
			return FALSE;
		}
		//La que llama al wait por primera vez es la tarea que va a esperar.
		//Es volver a hacer otra vez lo mismo, mejor ser claros.
		ev->state = EVENT_WAIT;
		break;
	case EVENT_SET:
		if((running_task_index > task_count) || (ev->task_waiting != running_task_index)){
			//Esta es una condición imposible ya que una vez que una tarea que hizo el primer wait
			//no puede cambiar. Es restrictivo
			os_error_hook();
		}
		ev->state = EVENT_WAIT;
		//Habilitamos una sección donde el Systick nos puede cambiar el estado de las tareas
		disable_sys_tick_irq();
		task_list[running_task_index].event_waiting = TRUE;
		task_list[running_task_index].state 				= TASK_SLEEPING;
		enable_sys_tick_irq();
		break;
	default:
		//Si estamos en un estado que no corresponde claramente algo salio muy mal
		os_error_hook();
		break;
	}
	//Cuando haemos un wait directamente le decimos al OS que hago un scheduler
	do_scheduler();
	return TRUE;
}

bool_t os_event_set	(os_event_handler_t event){
	event_t*	ev = (event_t*)(event);
	task_priority_t priority;
	switch(ev->state){
	case EVENT_INIT:
		//En EVENT_INIT  directamente no hacemos nada porque no interesa para el caso
		//Pero devolvemos un FALSE para indicar que nada se destrabo.
		return FALSE;
		break;
	case EVENT_WAIT:
		//Actualizamos a que tarea vamos a enviar el evento
		priority = task_list[ev->task_waiting].priority;
		//Obivamente el evento de este es un event set // Devolvemos el control al SO
		//para que encole esta tarea.
		ev->state = EVENT_SET;
		disable_sys_tick_irq();
		//Pasamos la tarea a ready y que no espera ningun evento
		//Por ejemplo la tarea que se libera podría llamar a un delay
		task_list[ev->task_waiting].event_waiting = FALSE;
		task_list[ev->task_waiting].state 				= TASK_READY;
		//Ponemos la tarea en la cola que corresponde en el fondo, si hay mas tarea de la misma prioridad
		//Se ejecutaran primero estas tarea
		task_stack_push(&priority_queue[priority],ev->task_waiting);
		enable_sys_tick_irq();
		break;
	case EVENT_SET:
		//En este caso si damos un event set no hacemos directamente nada porque
		//hacer un set cuando esta seteado no debería hacer4 nada.
		//En resumen hacer 20 event set es lo mismo que hacer uno solo, mientras no hayan echo un wait
		//Si hay una llave mandar a poner la llaver 20 veces es equivalente a ponerla una vez porque
		//hay una sola llave.
		break;
	default:
		os_error_hook();
		break;
	}
	return TRUE;
	//No forzamos un cambio de contexto, ya que la tarea que llamo a esto puede seguir ejecutando
	//cosas -> Tal vez se podria hacer algo con las prioridades para ver si hacer o no un cambio
	//de contexto
}
