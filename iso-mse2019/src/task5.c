#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
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
		if(os_event_wait(tecla_1_event) == TRUE){
			/*Copio el arreglo para minimizar el tiempo de uso
			 * TODO:Aca vendria bien un mutex*/
			copy_tecla_array(tecla_array_copy);
			if(tecla_array_copy[TECLA_1_INDEX].tecla_liberada_event == TRUE){
				/*Escribo y apago el LED BLUE teniendo en cuenta el arreglo de datos*/
				gpioWrite(LEDB,ON);
				os_task_delay(tecla_array_copy[TECLA_1_INDEX].ticks_presionada);
				gpioWrite(LEDB,OFF);
			}
			else {
				uartWriteString(UART_USB,"ERROR TECLA");
			}
		}else {
			/*Espero por siempre aca asi me doy cuenta
			 * que fallo*/
			uartWriteString(UART_USB,"ERROR TASK 5");
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
