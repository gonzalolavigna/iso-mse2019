#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task4.h"
#include "task3.h"

uint32_t task4_stack[TASK4_STACK_SIZE_BYTES/4];


void* task4 (void* a){
	//Despues esto va a crecer para poder ver mas teclas lo hacemos solo por tiempo
	while(1){
		//Lo hacemos esperar 1s y le damos el give
		os_task_delay(1000);
		while(os_event_set(tecla_event)==FALSE);
		//Lo hacemos esperar 2s y le damos el give
		os_task_delay(2000);
		while(os_event_set(tecla_event)==FALSE);
		//Lo hacemos esperar 4s y le damos el give
		os_task_delay(4000);
		while(os_event_set(tecla_event)==FALSE);
	}
}

void* hook4 (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
