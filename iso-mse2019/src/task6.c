#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "sapi_circularBuffer.h"
#include "task6.h"


//Definicion para el stack de la tarea 6
uint32_t task6_stack[TASK6_STACK_SIZE_BYTES/4];

//Definicion del buffer circular para poder recibir los datos desde la UART
uint8_t  uart_buffer_array[256];
circularBuffer_t uart_buffer_sapi;

//Funcion para printear cuando se llena el buffer circular para recibir por UART
void uart_buffer_full_message(void *);


/*Inicializamos el buffer circular de la SAPI*/
void init_uart_circular_buffer(void){
	circularBuffer_Init(&uart_buffer_sapi,uart_buffer_array,256,sizeof(uint8_t));
	circularBufferFullBufferCallbackSet(&uart_buffer_sapi,uart_buffer_full_message);
}

void uart_buffer_full_message(void * no_usado){
	//Para que no se mezclean los mensajes y ejecute sola
	//estamos en un error asi que no importa mucho si bloque
	os_enter_critical();
	printf("UART CIRCULAR BUFFER FULL REVISAR THROUGHPUT\r\n");
	os_quit_critical();
}


void* task6	(void* a){
	uint8_t temp_data;
	while(1){
		if(circularBufferRead(&uart_buffer_sapi,&temp_data) == CIRCULAR_BUFFER_EMPTY){
			//Esta tarea la paramos por 100 ms para que otras hagan sus cosas/ solo se despierta
			//para ver si hay algo en el buffer circular y lo reenvia por la UART.
			os_task_delay(100);
		}
		else {
			//Hago un reenvio de los datos recibidos
			uartWriteByte(UART_USB,temp_data);
		}
	}
}

void* hook6 (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

