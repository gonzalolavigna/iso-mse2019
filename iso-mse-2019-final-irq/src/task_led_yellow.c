#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_yellow.h"
#include "task_print.h"
#include "utils.h"


uint32_t task_led_yellow_stack[TASK_LED_YELLOW_STACK_SIZE_BYTES/4];

void* task_yellow (void* a){
	gpioWrite(LED1,OFF);
	while(1){
		if(os_event_wait(event_led_yellow) == TRUE){
			gpioWrite(LED1,ON);
			os_task_delay(get_led_delay_time());
			gpioWrite(LED1,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED YELLOW\r\n");
		}
	}
}
void* hook_yellow (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

