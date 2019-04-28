#ifndef __TASK_LED_GREEN__
#define __TASK_LED_GREEN__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_LED_GREEN_STACK_SIZE_BYTES 512

extern uint32_t task_led_green_stack[];

void* task_green (void* a);
void* hook_green (void* p);

#ifdef __cplusplus
}
#endif

#endif
