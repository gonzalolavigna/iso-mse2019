#include <stdint.h>
#include "string.h"
#include "os.h"
#include "sapi.h"
#include "task3.h"


uint32_t task3_stack[TASK3_STACK_SIZE_BYTES/4];

void* task3 (void* a){
	uint32_t i;
	while(1){
		os_task_delay(20);
		gpioToggle(LED3);
	}
}

void* hook3 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
