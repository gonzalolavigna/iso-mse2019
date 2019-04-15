#include "os.h"
#include "os_delay.h"
#include <stdint.h>
#include "string.h"
#include "sapi.h"
#include "task_stack.h"

uint32_t ticks_running = 0;


void update_delay(void);


bool_t os_task_delay(uint32_t ticks) {
	if (ticks == 0) {
		/*TODO:Todavia no encontre motivos porque un delay generaría un false*/
		return TRUE;
	}
	//Habilitamos una sección donde el Systick nos puede cambiar el estado de las tareas
	disable_sys_tick_irq();
	/*La tarea que llama a esta función pasa a sleeping hasta que se venzan los sistick*/
	task_list[running_task_index].state = TASK_SLEEPING;
	task_list[running_task_index].reamaining_ticks = ticks;
	enable_sys_tick_irq();
	/*Llamamos al scheduler para que busque que proxima tarea ejecutar*/
	do_scheduler();
	return TRUE;
}

/*Con esta función obtengo cuantos ticks en 32 bits que estoy corriendo*/
/*La idea es medir tiempos absolutos*/
uint32_t os_get_ticks_running(void){
	return ticks_running;
}


void SysTick_Handler(void) {
	/*Cada vez que entramos al sistick aumentamos esta variable que nos indica
	 * cuantos ticks absoluto corrio el O.S*/
	/*Util para medir tiempos*/
	ticks_running++;
	/*Debuggin para medir cuanto lleva una atención del sistick*/
	gpioToggle(GPIO4);
	/*Las tarea bloqueada por delay las decremento*/
	update_delay();
	gpioToggle(GPIO4);
	/*Siempre llamamos al cheduler para que decida que hacer a continuación*/
	do_scheduler();
}

void update_delay(void) {
	int i;
	/*Hago un loop en todas las tareas y decremento el contador de tareas bloqueadas por delay*/
	for (i = 0; i < task_count; i++) {
		switch (task_list[i].state) {
		/*SI la tarea esta en ready o running sigue como esta*/
		case TASK_RUNNING:
		case TASK_READY:
			break;
		case TASK_SLEEPING:
				//Si estoy esperando un evento no hago el trabajo de bajar el contador
				//Si queremos implementar un timeout para los eventos directamente tenemos que cambiar el enfoque
				if(task_list[i].event_waiting == FALSE){
					if ((--(task_list[i].reamaining_ticks)) == 0) {
						//Pongo la tarea en ready y la pongo en la cola de prioridad que le corresponde
						task_list[i].state = TASK_READY;
						/*Tener en cuenta que el push la va a enviar al fondo de la cola*/
						task_stack_push(&priority_queue[task_list[i].priority], i);
					}
				}
			break;
		default:
			os_error_hook();
			break;
		}
	}
}

/*Estas funciones deshabilitan las IRQ de Sistick la uso cuando con en el entorno del O.S
 * hago uso de las variable correspondiente al estado de las tareas*/
/*Esto es menos violento de deshabilitar todas las IRQ*/
void enable_sys_tick_irq(void){
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

/*Cuando en el entorno del OS hago uso de las variable del O.S cuando termino de usar variable
 * del OS vuelvo a habilitar la interrupcion del SIstick*/
void disable_sys_tick_irq(void){
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}
