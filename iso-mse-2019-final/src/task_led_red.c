#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_red.h"
#include "task_print.h"
#include "utils.h"


uint32_t task_led_red_stack[TASK_LED_RED_STACK_SIZE_BYTES/4];

void* task_red (void* a){
	gpioWrite(LED2,OFF);
	while(1){
		if(os_event_wait(event_led_red) == TRUE){
			gpioWrite(LED2,ON);
			os_task_delay(get_led_delay_time());
			gpioWrite(LED2,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED RED\r\n");
		}
	}
}
void* hook_red(void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
