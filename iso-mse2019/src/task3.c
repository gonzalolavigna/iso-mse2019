#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task3.h"


//Estos datos se comparten externamente
uint32_t 						task3_stack[TASK3_STACK_SIZE_BYTES/4];
os_event_handler_t 	tecla_event;

void* task3 (void* a){
	uint32_t i;
	//Mensaje de bienvenida de la Tarea 3 para
	uartWriteString(UART_USB,"TASK 3-EVENTOS\r\n");
	gpioInit(GPIO2,GPIO_OUTPUT);
	gpioWrite(GPIO2,ON);
	while(1){

		if(os_event_wait(tecla_event) == TRUE){
			//Este evento va a provenir de la Tarea 4 Hacemos togglear un GPIO para poder
			//verlo con el analizador logico
			gpioToggle(GPIO2);
			uartWriteString(UART_USB,"EVENTO TECLA DETECTADO\r\n");
			printf("PAPUCHO\r\n");
			gpioToggle(GPIO2);
		} else {
			uartWriteString(UART_USB,"PROBLEMA ESPERANDO EVENTO\r\n");
		}
	}
	return (void *)TRUE;
}

void* hook3 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
