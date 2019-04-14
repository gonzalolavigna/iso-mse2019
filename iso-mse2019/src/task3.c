#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task3.h"


uint32_t task3_stack[TASK3_STACK_SIZE_BYTES/4];

void* task3 (void* a){
	uint32_t i;
	gpioInit(GPIO2,GPIO_OUTPUT);
	gpioWrite(GPIO2,OFF);
	while(1){

		uartWriteString(UART_USB,"TASK 3 TESTING TASK 3 TASK 3-FPU CONTEXT\r\n");
		gpioWrite(GPIO2,ON);
		os_task_delay(2000);
		gpioWrite(GPIO2,OFF);
		//gpioToggle(LED3);
		//gpioToggle(GPIO2);
	}
}

void* hook3 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
