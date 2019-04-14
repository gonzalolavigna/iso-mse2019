#ifndef __TASK1__
#define __TASK1__

#ifdef __cplusplus
extern "C" {
#endif

#define TASK1_STACK_SIZE_BYTES 512

extern uint32_t task1_stack[];

void* task1 (void* a);
void* hook1 (void* p);


#ifdef __cplusplus
}
#endif

#endif

