#include "main.h"         // <= Su propia cabecera
#include "board.h"
#include "sapi.h"
#include "string.h"

#define STACK_SIZE_B 512

typedef void * (* task_type)(void *);

uint32_t stack_1 [STACK_SIZE_B/4];
uint32_t stack_2 [STACK_SIZE_B/4];

uint32_t stack_1_pointer;
uint32_t stack_2_pointer;

uint32_t current_task;


void * task1 (void * arg){
	uint32_t tick_count = 250;
	while(1){
		if(tick_count == 0){
			tick_count = 250;
			gpioToggle(LED1);
		}
		else
			tick_count--;
		__WFI();
	}
	return NULL;
}

void * task2 (void * arg){
	uint32_t tick_count = 500;
	while(1){
		if(tick_count == 0){
			tick_count = 500;
			gpioToggle(LED2);
		}
		else
			tick_count--;
		__WFI();
	}
	return NULL;
}


uint32_t get_next_context (uint32_t current_sp){

	uint32_t next_sp;

	switch(current_task){
	case 0:
		/*Vere que hacer con el contexto inicial*/
		next_sp = stack_1_pointer;
		current_task = 1;
		break;
	case 1:
		stack_1_pointer = current_sp;
		next_sp =stack_2_pointer ;
		current_task = 2;
		break;
	case 2:
		stack_2_pointer = current_sp;
		next_sp =stack_1_pointer;
		current_task = 1;
		break;
	default:
		while(1){
			__WFI();
		}
		break;
	}
	return next_sp;
}

void task_return_hook (void * ret_val){
	while (1){
		__WFI();
	}
}

void init_stack (	uint32_t stack[],
					uint32_t stack_byte_size,
					uint32_t *sp,
					task_type entry_point,
					void * arg){

	bzero(stack,stack_byte_size);

	stack[stack_byte_size/4-1] = 1 << 24;				/*XPSR.T = 1*/
	stack[stack_byte_size/4-2] = (uint32_t) entry_point; 			/*PC*/
	stack[stack_byte_size/4-3] = (uint32_t) task_return_hook;		/*LR*/
	stack[stack_byte_size/4-8] = (uint32_t) arg; 					/*R0*/
	stack[stack_byte_size/4-9] = 0xFFFFFFF9;			/*LR IRQ*/

	*sp =(uint32_t) &(stack[stack_byte_size/4-17]); //Corri el stack point 8 lugares hacia donde crece la pila
}






int main (void){

	// Inicializar y configurar la plataforma
	init_stack(stack_1,STACK_SIZE_B,&stack_1_pointer,task1, (void *)0x11223344);
	init_stack(stack_2,STACK_SIZE_B,&stack_2_pointer,task2, (void *)0x11223344);

	//Board_Init();
	//SystemCoreClockUpdate();
	//SysTick_Config(SystemCoreClock / 1000);
	boardConfig();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	while(1){
		__WFI();
	}
}

