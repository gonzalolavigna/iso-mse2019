#ifndef __TASK_LED_RED__
#define __TASK_LED_RED__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_LED_RED_STACK_SIZE_BYTES 512

extern uint32_t task_led_red_stack[];

void* task_red (void* a);
void* hook_red (void* p);

#ifdef __cplusplus
}
#endif

#endif
