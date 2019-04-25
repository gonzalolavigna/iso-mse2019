#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task_led_blue.h"
#include "task_print.h"
#include "utils.h"


uint32_t task_led_blue_stack[TASK_LED_BLUE_STACK_SIZE_BYTES/4];

void* task_blue (void* a){
	gpioWrite(LEDB,OFF);
	while(1){
		if(os_event_wait(event_led_blue) == TRUE){
			gpioWrite(LEDB,ON);
			os_task_delay(get_led_delay_time());
			gpioWrite(LEDB,OFF);
		}else {
			uartWriteString(UART_USB,"ERROR EVENTO LED BLUE\r\n");
		}
	}
}
void* hook_blue (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

