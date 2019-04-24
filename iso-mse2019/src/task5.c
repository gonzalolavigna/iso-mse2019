#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
//Aca esta declarado el mutex
#include "task3.h"
#include "task4.h"
#include "task5.h"



uint32_t task5_stack[TASK5_STACK_SIZE_BYTES/4];
os_event_handler_t 	tecla_1_event;

//TODO: Este arreglo de 4 eventos de teclas
static debounce_data_t tecla_array_copy[4];

void* task5	(void* a){
	os_task_delay(100);
	gpioWrite(LEDB,OFF);
	while(1){
		if(os_event_wait(tecla_event) == TRUE){
			/*Copio el arreglo para minimizar el tiempo de uso
			 * TODO:Aca vendria bien un mutex*/
			copy_tecla_array(tecla_array_copy);
			if(tecla_array_copy[TECLA_1_INDEX].tecla_liberada_event == TRUE){
				/*Escribo y apago el LED BLUE teniendo en cuenta el arreglo de datos*/
				//Esto deberia ocasionar el mismo uso de una sección critica por 3 y 5 tienen misma prioridad
				os_mutex_lock(uart_mutex);
				printf("TASK 5:TECLA %d PRESIONADA POR:%d TICKS\r\n",1,tecla_array_copy[TECLA_1_INDEX].ticks_presionada);
				os_mutex_unlock(uart_mutex);
				gpioWrite(LEDB,ON);
				os_task_delay(tecla_array_copy[TECLA_1_INDEX].ticks_presionada);
				gpioWrite(LEDB,OFF);
			}
		}else {
			/*Espero por siempre aca asi me doy cuenta
			 * que fallo*/
			uartWriteString(UART_USB,"ERROR EVENTO TASK 5");
		}
	}
	return (void *)TRUE;
}


void* hook5 (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
