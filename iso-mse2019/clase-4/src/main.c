#include "main.h"         // <= Su propia cabecera
#include "board.h"
#include "sapi.h"
#include "string.h"

#define STACK_SIZE_B 512
typedef enum {
	RUNNING, BLOCKED
} task_state_t;

typedef enum {
	MAIN_STACK = 0, TASK1, TASK2, IDLE_TASK
} task_index_t;

typedef void * (*task_type)(void *);


void schedule(void);

uint32_t stack_1[STACK_SIZE_B / 4];
uint32_t stack_2[STACK_SIZE_B / 4];
uint32_t stack_idle[STACK_SIZE_B / 4];

uint32_t stack_1_pointer, stack_2_pointer, stack_idle_pointer;

uint32_t task1_delay, task2_delay;
task_state_t task1_state, task2_state, idle_state;

task_index_t current_task = MAIN_STACK;

void task_delay(uint32_t delay) {
	if (delay == 0) {
		return;
	}
	switch (current_task) {
	case 1:
		task1_state = BLOCKED;
		task1_delay = delay;
		break;
	case 2:
		task2_state = BLOCKED;
		task2_delay = delay;
		break;
	default:
		while (1) {
			__WFI();
		}
	}
	//Sera esta la funcion para esperar hasta que termine. La idea seria
	//llamar directamente al scheduler.
	__WFI();
}

void update_tasks(void) {
	if (task1_state == BLOCKED) {
		if (--task1_delay == 0) {
			task1_state = RUNNING;
		}
	}
	if (task2_state == BLOCKED) {
		if (--task2_delay == 0) {
			task2_state = RUNNING;
		}
	}
}

uint32_t get_next_context(uint32_t current_sp) {

	uint32_t next_sp;
	update_tasks();
	switch (current_task) {
	case MAIN_STACK:
		/*Vere que hacer con el contexto inicial*/
		next_sp = stack_1_pointer;
		current_task = 1;
		break;
	case TASK1:
		stack_1_pointer = current_sp;
		if (task2_state == RUNNING) {
			next_sp = stack_2_pointer;
			current_task = TASK2;
		} else if (task1_state == RUNNING) {
			next_sp = stack_1_pointer;
			current_task = TASK1;
		} else {
			next_sp = stack_idle_pointer;
			current_task = IDLE_TASK;
		}
		break;
	case TASK2:
		stack_2_pointer = current_sp;
		if (task1_state == RUNNING) {
			next_sp = stack_1_pointer;
			current_task = TASK1;
		} else if (task2_state == RUNNING) {
			next_sp = stack_2_pointer;
			current_task = TASK2;
		} else {
			next_sp = stack_idle_pointer;
			current_task = IDLE_TASK;
		}
		break;
	case IDLE_TASK:
		stack_idle_pointer = current_sp;
		if (task1_state == RUNNING) {
			next_sp = stack_1_pointer;
			current_task = TASK1;
		} else if (task2_state == RUNNING) {
			next_sp = stack_2_pointer;
			current_task = TASK2;
		} else {
			next_sp = stack_idle_pointer;
			current_task = IDLE_TASK;
		}
		break;
	default:
		while (1) {
			__WFI();
		}
		break;
	}
	return next_sp;
}

void task_return_hook(void * ret_val) {
	while (1) {
		__WFI();
	}
}

//Tarea IDLE para irme en caso que tenga algun incoveniente
void * idle_task(void * arg) {
	while (1) {
		__WFI();
	}
}

void * task1(void * arg) {
	uint32_t tick_count = 2000;
	while (1) {
		gpioToggle(LED1);
		task_delay(tick_count);
	}
	return NULL;
}

void * task2(void * arg) {
	uint32_t tick_count = 4000;
	while (1) {
		gpioToggle(LED2);
		task_delay(tick_count);
	}
	return NULL;
}

void init_stack(uint32_t stack[], uint32_t stack_byte_size, uint32_t *sp,
    task_type entry_point, void * arg) {

	bzero(stack, stack_byte_size);

	stack[stack_byte_size / 4 - 1] = 1 << 24; /*XPSR.T = 1*/
	stack[stack_byte_size / 4 - 2] = (uint32_t) entry_point; /*PC*/
	stack[stack_byte_size / 4 - 3] = (uint32_t) task_return_hook; /*LR*/
	stack[stack_byte_size / 4 - 8] = (uint32_t) arg; /*R0*/
	stack[stack_byte_size / 4 - 9] = 0xFFFFFFF9; /*LR IRQ*/

	*sp = (uint32_t) &(stack[stack_byte_size / 4 - 17]); //Corri el stack point 8 lugares hacia donde crece la pila
}


void schedule(void){
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

}

void SysTick_Handler (){

	//NVIC_SetPendingIRQ(PendSV_IRQn);
	schedule();
	//__ISB();
	//__DSB();
	//SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


int main(void) {

	//NVIC_ClearPendingIRQ()
	//NVIC_EnableIRQ(PendSV_IRQn);

	// Inicializar y configurar la plataforma
	init_stack(stack_1, STACK_SIZE_B, &stack_1_pointer, task1,
	    (void *) 0x11223344);
	init_stack(stack_2, STACK_SIZE_B, &stack_2_pointer, task2,
	    (void *) 0x11223344);
	init_stack(stack_idle, STACK_SIZE_B, &stack_idle_pointer, idle_task,
	    (void *) 0x11223344);

	task1_state = RUNNING;
	task2_state = RUNNING;
	idle_state = RUNNING;

	//Board_Init();
	//SystemCoreClockUpdate();
	//SysTick_Config(SystemCoreClock / 1000);
	boardConfig();
	SystemCoreClockUpdate();
	NVIC_SetPriority(PendSV_IRQn,(1 <<__NVIC_PRIO_BITS) -1);
	SysTick_Config(SystemCoreClock / 1000);

	while (1) {
		__WFI();
	}
}

