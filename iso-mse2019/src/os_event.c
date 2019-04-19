#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task_stack.h"

event_t 	event_list[MAX_EVENT_COUNT];
mutex_t 	mutex_list[MAX_MUTEX_COUNT];
uint32_t 	event_count = 0;
uint32_t 	mutex_count = 0;

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
	//Si queremos crear un evento mas de lo que podemos directamente devolvemos NULL
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
	/*EL event handler es un void* esto es de FreeRTOS*/
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
		if((running_task_index > task_count)){
			//Esta es una condición imposible ya que una vez que una tarea que hizo el primer wait
			//no puede cambiar. Es restrictivo
			os_error_hook();
		}
		ev->state = EVENT_WAIT;
		ev->task_waiting = running_task_index;
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
	/*EL event handler es un void* esto es de FreeRTOS*/
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
	/*TODO:Evaluar si no es necesario hacer un cambio de contexto esta el fundamento:
	 * Si la tarea a la que vuelvo es de mayor prioridad la estoy penalizando
	 * SI la tarea a la que vuelvo es de menor prioridad esto penalizando a la que hace el event set*/
	return TRUE;
	//No forzamos un cambio de contexto, ya que la tarea que llamo a esto puede seguir ejecutando
	//cosas -> Tal vez se podria hacer algo con las prioridades para ver si hacer o no un cambio
	//de contexto
}



void os_mutex_init_array(void){
	int i;
	for(i=0;i<MAX_MUTEX_COUNT;i++){
		mutex_list[i].mutex_state= MUTEX_UNLOCK;
		//Las tareas que apunta las ponemos en el maximo posible + 1
		mutex_list[i].event = NULL;
	}
}

os_mutex_handler_t os_mutex_init(void){
	os_event_handler_t ev_handler;
	os_mutex_handler_t mutex_handler;

	if(mutex_count >= MAX_MUTEX_COUNT){
		return NULL;
	}
	//Obivamente un mutex utiliza un semaforo
	//Si no pudo crear el evento directamente devuelvo NULL
	if((ev_handler=os_event_init())==NULL){
		return NULL;
	}
	//El evento del mutex queda asignado
	mutex_list[mutex_count].event = ev_handler;
	//Todos los mutex arrancan en unlock
	mutex_list[mutex_count].mutex_state = MUTEX_UNLOCK;
	//Aranca la FSM de un lado distinto ya que el evento esta en set por defecto
	//Hace un lock me da directamente al estado de wait en la proxima vez que pase
	((event_t*)mutex_list[mutex_count].event)->state = EVENT_SET;
	mutex_handler = &mutex_list[mutex_count];
	mutex_count++;
	return mutex_handler;

}


bool_t	os_mutex_lock 	(os_mutex_handler_t mutex){
	mutex_t* local_mutex= (mutex_t*)mutex;
	switch(local_mutex->mutex_state){
	case MUTEX_LOCK:
		//Cuando el mutex esta lock hay que esperar que alguien lo libere por lo tanto
		//ponemos a la tarea que lo llama a dormir
		if(os_event_wait(local_mutex->event) == FALSE){
			uartWriteString(UART_USB,"OS:ERROR EN MUTEX LOCK");
		}
		break;
	case MUTEX_UNLOCK:
		local_mutex->mutex_state = MUTEX_LOCK;
		break;
	default:
		os_error_hook();
		break;
	}
	return TRUE;
}


bool_t os_mutex_unlock (os_mutex_handler_t mutex){
	mutex_t* local_mutex= (mutex_t*)mutex;
	switch(local_mutex->mutex_state){
	case MUTEX_LOCK:
		//Como esta en lock mandamos el evento a la tarea
		if(os_event_set(local_mutex->event)== FALSE){
			uartWriteString(UART_USB,"OS:ERROR EN MUTEX UNLOCK");
		}
		local_mutex->mutex_state = MUTEX_UNLOCK;
		break;
	case MUTEX_UNLOCK:
		//Hacer un unlock cuando esta unlock no importa
		//ya que esta seria un caso en el que se lockea algo y desppues se unlockea
		//sin ningun tarea pedir acceso a la sección crítica
		break;
	default:
		os_error_hook();
		break;
	}
	return TRUE;
}



