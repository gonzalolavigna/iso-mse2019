#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task3.h"
#include "task4.h"


//Estos datos se comparten externamente
uint32_t 						task3_stack[TASK3_STACK_SIZE_BYTES/4];
os_event_handler_t 	tecla_event;
os_mutex_handler_t 	uart_mutex;

//TODO: Este arreglo de 4 eventos de teclas
static debounce_data_t tecla_array_copy[4];

void* task3 (void* a){
	uint32_t i;
	//Mensaje de bienvenida de la Tarea 3 para
	uartWriteString(UART_USB,"TASK 3-EVENTOS-TECLAS-UART-FLOAT-REMOVE-ITEM-FLOAT-TO-STR-PUT-TASK-TO-SLEEP-READY\r\n");
	gpioInit(GPIO2,GPIO_OUTPUT);
	gpioWrite(GPIO2,ON);
	while(1){
		if(os_event_wait(tecla_event) == TRUE){
			copy_tecla_array(tecla_array_copy);
			for(i=0;i<4;i++){
				if(tecla_array_copy[i].tecla_liberada_event == TRUE){
					//Entramos a una seccion donde compartimos el mismo recurso que es la UART
					os_mutex_lock(uart_mutex);
					printf("TASK 3:TECLA %d PRESIONADA POR:%d TICKS\r\n",i+1,tecla_array_copy[i].ticks_presionada);
					os_mutex_unlock(uart_mutex);
				}
			}

		} else {
			uartWriteString(UART_USB,"PROBLEMA ESPERANDO EVENTO\r\n");
		}
		gpioToggle(GPIO2);
	}
	return (void *)TRUE;
}

void* hook3 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
