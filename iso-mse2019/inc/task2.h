#ifndef __TASK2__
#define __TASK2__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK2_STACK_SIZE_BYTES 512

extern uint32_t task2_stack[];

void* task2 (void* a);
void* hook2 (void* p);


#ifdef __cplusplus
}
#endif

#endif
