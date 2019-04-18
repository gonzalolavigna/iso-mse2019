#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task1.h"


//Estos datos se comparten externamente
uint32_t 						task1_stack[TASK1_STACK_SIZE_BYTES/4];
os_event_handler_t 	irq_emulator_event;


void* task1 (void* a){
	gpioInit(GPIO0,GPIO_OUTPUT);
	gpioWrite(GPIO0,OFF);
	while(1){
		gpioToggle(GPIO0);
		gpioToggle(GPIO0);
		gpioToggle(LED1);
		//LO vemos con analizador lógico y hacemos el retardo de 6 ms
		os_task_delay(6);
		os_event_set(irq_emulator_event);
	}
}


void* hook1 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
