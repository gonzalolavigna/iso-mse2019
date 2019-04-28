#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_yellow.h"
#include "task_print.h"
#include "utils.h"

//Stack de la tarea
uint32_t task_led_yellow_stack[TASK_LED_YELLOW_STACK_SIZE_BYTES/4];

//Tarea para ser llamada en nuestro OS
void* task_yellow (void* a){
	gpioWrite(LED1,OFF);
	while(1){
		//Espero hasta que venga un evento que me indique prender una tecla
		if(os_event_wait(event_led_yellow) == TRUE){
			//Prendemos y apagamos un led la cantidad de tiempo necesaria pasada por el entorno de task print.
			gpioWrite(LED1,ON);
			os_task_delay(get_led_delay_time());
			gpioWrite(LED1,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED YELLOW\r\n");
		}
	}
}

//No implementado en nuestro OS.
void* hook_yellow (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

