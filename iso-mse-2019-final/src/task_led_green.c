#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_green.h"
#include "task_print.h"
#include "utils.h"


uint32_t task_led_green_stack[TASK_LED_GREEN_STACK_SIZE_BYTES/4];

void* task_green (void* a){
	gpioWrite(LED3,OFF);
	while(1){
		if(os_event_wait(event_led_green) == TRUE){
			gpioWrite(LED3,ON);
			os_task_delay(get_led_delay_time());
			gpioWrite(LED3,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED GREEN\r\n");
		}
	}
}
void* hook_green (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

