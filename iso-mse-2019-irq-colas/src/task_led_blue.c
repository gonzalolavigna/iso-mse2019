#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_blue.h"
#include "task_print.h"
#include "utils.h"

//Stack de la tarea
uint32_t task_led_blue_stack[TASK_LED_BLUE_STACK_SIZE_BYTES/4];

//Tarea para ser llamada en nuestro OS
void* task_blue (void* a){
	gpioWrite(LEDB,OFF);
	while(1){
		//Espero hasta que venga un evento que me indique prender una tecla
		if(os_event_wait(event_led_blue) == TRUE){
			gpioWrite(LEDB,ON);
			//Prendemos y apagamos un led la cantidad de tiempo necesaria pasada por el entorno de task print.
			os_task_delay(get_led_delay_time());
			gpioWrite(LEDB,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED BLUE\r\n");
		}
	}
}

//No implementado en nuestro OS.
void* hook_blue (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

