#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "sapi.h"
#include "task1.h"


uint32_t task1_stack[TASK1_STACK_SIZE_BYTES/4];

void* task1 (void* a){
	uint32_t i;
	gpioInit(GPIO0,GPIO_OUTPUT);
	while(1){
		os_task_delay(500);
		gpioToggle(LED1);
		gpioToggle(GPIO0);
	}
}

void* hook1 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
