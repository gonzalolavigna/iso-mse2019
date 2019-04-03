#include <stdint.h>
#include "string.h"
#include "os.h"
#include "sapi.h"
#include "task2.h"


uint32_t task2_stack[TASK2_STACK_SIZE_BYTES/4];

void* task2 (void* a){
	uint32_t i;
	while(1){
		os_task_delay(10);
		gpioToggle(LED2);
	}
}

void* hook2 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
