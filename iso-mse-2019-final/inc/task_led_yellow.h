#ifndef __TASK_LED_YELLOW__
#define __TASK_LED_YELLOW__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_LED_YELLOW_STACK_SIZE_BYTES 512

extern uint32_t task_led_yellow_stack[];

void* task_yellow (void* a);
void* hook_ (void* p);

#ifdef __cplusplus
}
#endif

#endif
