#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_green.h"
#include "task_print.h"
#include "utils.h"

//Stack de la tarea
uint32_t task_led_green_stack[TASK_LED_GREEN_STACK_SIZE_BYTES/4];

//Tarea para ser llamada en nuestro OS
void* task_green (void* a){
	gpioWrite(LED3,OFF);
	while(1){
		//Espero hasta que venga un evento que me indique prender una tecla
		if(os_event_wait(event_led_green) == TRUE){
			gpioWrite(LED3,ON);
			//Prendemos y apagamos un led la cantidad de tiempo necesaria pasada por el entorno de task print.
			os_task_delay(get_led_delay_time());
			gpioWrite(LED3,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED GREEN\r\n");
		}
	}
}

//No implementado en nuestro OS.
void* hook_green (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

