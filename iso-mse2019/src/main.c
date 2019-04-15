#include "sapi.h"
#include "os.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"
#include <string.h>


int main (void){
	uartInit(UART_USB,115200);
	uartWriteString(UART_USB,"OS-ISO MSE 2019 Gonzalo Lavigna\r\n");
	os_queue_init();

	//Creo el evento de la tecla antes de lanzar el OS --> si es NULL me quedo esperando
	if((tecla_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:TECLA\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	os_task_create(task1_stack,TASK1_STACK_SIZE_BYTES,task1,HIGH_PRIORITY	,(void*)0x11111111);
	os_task_create(task2_stack,TASK2_STACK_SIZE_BYTES,task2,HIGH_PRIORITY	,(void*)0x22222222);
	os_task_create(task4_stack,TASK4_STACK_SIZE_BYTES,task4,HIGH_PRIORITY	,(void*)0x44444444);
	os_task_create(task3_stack,TASK3_STACK_SIZE_BYTES,task3,LOW_PRIORITY	,(void*)0x33333333);

	os_init();

	while(1){
		__WFI();
	}

	return TRUE;
}
