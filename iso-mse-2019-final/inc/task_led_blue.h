#ifndef __TASK_LED_BLUE__
#define __TASK_LED_BLUE__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_LED_BLUE_STACK_SIZE_BYTES 512

extern uint32_t task_led_blue_stack[];

void* task_blue (void* a);
void* hook_blue (void* p);

#ifdef __cplusplus
}
#endif

#endif
