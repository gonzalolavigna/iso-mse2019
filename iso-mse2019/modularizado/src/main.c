#include "sapi.h"
#include "os.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"

extern uint32_t task1_stack[];
extern uint32_t task2_stack[];
extern uint32_t task3_stack[];

int main (void){

	os_task_create(task1_stack,TASK1_STACK_SIZE_BYTES,task1,HIGH_PRIORITY,(void*)0x11111111);
	os_task_create(task2_stack,TASK2_STACK_SIZE_BYTES,task2,HIGH_PRIORITY,(void*)0x11111111);
	os_task_create(task3_stack,TASK3_STACK_SIZE_BYTES,task3,HIGH_PRIORITY,(void*)0x11111111);

	os_init();

	while(1){
		__WFI();
	}

	return TRUE;
}
