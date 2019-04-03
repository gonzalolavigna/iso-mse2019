#include "../../../iso-mse2019/modularizado/inc/os.h"
#include <stdint.h>
#include "string.h"
#include "sapi.h"

#define MAX_TASK_COUNT 10
//Ponemos un tamaño minimo del stack sin este no podemos crear un stack
//para los registros del procesador
#define STACK_MIN_SIZE 20

#define IDLE_TASK_SIZE_BYTES 512

/*Variable globales del OS*/

task_context_t 	task_list[MAX_TASK_COUNT];
task_context_t 	idle_contex;

uint32_t	task_count 					= 0;
uint32_t 	running_task_index	= 0;
uint32_t 	idle_task_stack[IDLE_TASK_SIZE_BYTES/4];
os_state_t 	os_state 					= OS_INIT;

void 		init_task_stack	(	uint32_t stack[],
								uint32_t stack_size_bytes,
								uint32_t *stack_pointer,
								task_type_f entry_point,
								void * arg );
void* 		idle_task 		( 	void * arg);
void 		os_error_hook 	(	void);
void 		do_scheduler	(	void);
void 		task_return_hook(	void * ret_val);
void 		update_delay	(	void);


/*Declaracion de funciones externas utilizadas por el SO*/

bool_t os_task_create	(	uint32_t stack[],uint32_t stack_size_bytes,task_type_f entry_point,
							task_priority_t priority, void * arg ){

	//Reviso que no tenga mas tareas de las que puedo manejar.
	//Reviso que el stack que me estan pasando no sea menor a 20 porque sino no
	//puedo crear un stack para el tamaño de los registros
	//Reviso que el stack sea divisible por 4, ya que las cuentas se hacen con ese
	//valor
	if(task_count >= MAX_TASK_COUNT || stack_size_bytes < 20 || stack_size_bytes%4)
		return FALSE;

	task_list[task_count].state 					= TASK_READY;
	task_list[task_count].priority 					= HIGH_PRIORITY;
	task_list[task_count].reamaining_ticks 			= 0;
	task_list[task_count].initial_stack_pointer 	= stack;
	task_list[task_count].stack_size_bytes			= stack_size_bytes;
	task_list[task_count].task_index 				= task_count;

	//Inicializo el stack y ya queda actualizado el stack pointer al lugar donde tengo el stack
	//para ejecutar la tarea
	init_task_stack(stack,
									stack_size_bytes,
									&task_list[task_count].stack_pointer,
									entry_point,
									arg);

	//Incremento la cantidad de tarea en el scheduler
	task_count++;

	return TRUE;
}

bool_t os_task_delay	(uint32_t ticks){
	if(ticks == 0){
		return TRUE;
	}
	task_list[running_task_index].state 				= TASK_SLEEPING;
	task_list[running_task_index].reamaining_ticks 		= ticks;
	do_scheduler();
	return TRUE;
}


void init_task_stack(	uint32_t stack[],uint32_t stack_size_bytes,uint32_t *stack_pointer,
						task_type_f entry_point, void * arg ) {

	bzero(stack,stack_size_bytes);

	stack[stack_size_bytes / 4 - 1] = 1 << 24; 						/*XPSR.T = 1*/
	stack[stack_size_bytes / 4 - 2] = (uint32_t) entry_point; 		/*PC*/
	stack[stack_size_bytes / 4 - 3] = (uint32_t) task_return_hook; 	/*LR*/
	stack[stack_size_bytes / 4 - 8] = (uint32_t) arg; 				/*R0*/
	stack[stack_size_bytes / 4 - 9] = 0xFFFFFFF9; 					/*LR IRQ*/ 

	*stack_pointer = (uint32_t) &(stack[stack_size_bytes/4 - 17]);   //Corri el stack point 8 lugares hacia donde crece la pila

}

bool_t os_init(void){
	boardConfig();
	SystemCoreClockUpdate();
	NVIC_SetPriority(PendSV_IRQn,(1 <<__NVIC_PRIO_BITS) -1);
	SysTick_Config(SystemCoreClock / 1000);

	//Configuracion de la tarea IDLE
	idle_contex.state 					= TASK_READY;
	idle_contex.priority 				= IDLE_PRIORITY;
	idle_contex.reamaining_ticks 		= 0;
	idle_contex.initial_stack_pointer	= idle_task_stack;
	idle_contex.stack_size_bytes		= IDLE_TASK_SIZE_BYTES;
	idle_contex.task_index 							= 0xFFFFFFFF;
	init_task_stack(	idle_task_stack,
						IDLE_TASK_SIZE_BYTES,
						&idle_contex.stack_pointer,
						idle_task,
						(void *) 0x99999999);

	return TRUE;
}

void* idle_task 		(void * arg){
	while (1){
		__WFI();
	}
	return 0;
}
void task_return_hook(void * ret_val) {
	while (1) {
		__WFI();
	}
}

void os_error_hook (void){
	while(1){
		__WFI();
	}
}


uint32_t get_next_context(uint32_t current_sp){
	uint8_t 	i;
	bool_t 	  task_hit = FALSE;
	uint32_t 	next_stack_pointer;
	uint32_t 	task_index;

	switch (os_state){
	case OS_INIT:
		for(i=0; i < task_count;i++){
			task_index = i;
			switch (task_list[task_index].state){
			case TASK_RUNNING:
				os_error_hook();
				break;
			case TASK_SLEEPING:
				break;
			case TASK_READY:
				running_task_index = task_index;
				task_hit = TRUE;
				break;
			default:
				os_error_hook();
				break;
			}
			if(task_hit == TRUE){
				os_state = OS_RUNNING_TASK;
				next_stack_pointer = task_list[task_index].stack_pointer;
				//Si encontra la tarea no la tengo que seguir buscando.
				break;
			}
			else{
				os_state = OS_RUNNING_IDLE;
				next_stack_pointer = idle_contex.stack_pointer;
				//Sigo haciendo el correspondiente loop
			}
		}
		break;
	case OS_RUNNING_TASK:
		//Asigno a la tarea que sea esta ejecutando el valor del stack actual
		//por si hay que saltar a otra tarea.
		task_list[running_task_index].stack_pointer = current_sp;
		task_list[running_task_index].state = TASK_READY;
		for(i= 0; i < task_count; task_count){
			task_index = ((i+1)%task_count);
			switch(task_list[task_index].state){
			case TASK_RUNNING:
				task_list[task_index].state = TASK_READY;
				break;
			case TASK_READY:
				task_list[task_index].state = TASK_RUNNING;
				task_hit = TRUE;
				break;
			case TASK_SLEEPING:
				break;
			default:
				os_error_hook();
				break;
			}
			if(task_hit == TRUE){
				os_state = OS_RUNNING_TASK;
				next_stack_pointer = task_list[task_index].stack_pointer;
				//Si lo encontre no lo tengo que seguir buscando.
				break;
			}
			else{
				os_state = OS_RUNNING_IDLE;
				next_stack_pointer = idle_contex.stack_pointer;
			}
		}
		break;
	case OS_RUNNING_IDLE:
		idle_contex.stack_pointer = current_sp;
		for(i=0;i<task_count;i++){
			task_index = ((i+1)%task_count);
			switch(task_list[task_index].state){
			case TASK_RUNNING:
				task_list[task_index].state = TASK_READY;
				break;
			case TASK_READY:
				task_list[task_index].state = TASK_RUNNING;
				task_hit = TRUE;
				break;
			case TASK_SLEEPING:
				break;
			default:
				os_error_hook();
				break;
			}
			if(task_hit == TRUE){
				os_state = OS_RUNNING_TASK;
				next_stack_pointer = task_list[task_index].stack_pointer;
				//Si lo encontre no lo tengo que seguir buscando.
				break;
			}
			else{
				os_state = OS_RUNNING_IDLE;
				next_stack_pointer = idle_contex.stack_pointer;
			}
		}
		break;
	default:
		os_error_hook();
		break;
	}

	return next_stack_pointer;
}

void do_scheduler(void){
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void SysTick_Handler (void){
	//En el systick handler llamamos al scheduler haciendo la interrupcion de pendsv.
	update_delay();
	do_scheduler();
}

void update_delay(void){
	int i;
	for(i=0; i<task_count;i++){
		switch(task_list[i].state){
		case TASK_RUNNING :
		case TASK_READY   :
			break;
		case TASK_SLEEPING:
			if((--(task_list[i].reamaining_ticks))== 0){
				task_list[i].state = TASK_READY;
			}
			break;
		default:
			os_error_hook();
			break;
		}
	}
}
