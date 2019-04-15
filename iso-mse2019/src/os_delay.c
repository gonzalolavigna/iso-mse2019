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
		return TRUE;
	}
	//Habilitamos una sección donde el Systick nos puede cambiar el estado de las tareas
	disable_sys_tick_irq();
	task_list[running_task_index].state = TASK_SLEEPING;
	task_list[running_task_index].reamaining_ticks = ticks;
	enable_sys_tick_irq();
	do_scheduler();
	return TRUE;
}

uint32_t os_get_ticks_running(void){
	return ticks_running;
}


void SysTick_Handler(void) {
	ticks_running++;
	gpioToggle(GPIO4);
//En el systick handler llamamos al scheduler haciendo la interrupcion de pendsv.
	update_delay();
	gpioToggle(GPIO4);
	do_scheduler();
}

void update_delay(void) {
	int i;
	for (i = 0; i < task_count; i++) {
		switch (task_list[i].state) {
		case TASK_RUNNING:
		case TASK_READY:
			break;
		case TASK_SLEEPING:
				//Si estoy esperando un evento no hago el trabajo de bajar el contador
				//Si queremos implementar un timeout para los eventos directamente tenemos
				//que cambiar el enfoque
				if(task_list[i].event_waiting == FALSE){
					if ((--(task_list[i].reamaining_ticks)) == 0) {
						//Pongo la tarea en ready y la pongo en la cola de prioridad que le corresponde
						task_list[i].state = TASK_READY;
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

void enable_sys_tick_irq(void){
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
}

void disable_sys_tick_irq(void){
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

