#ifndef __TASK1__
#define __TASK1__

#include <stdint.h>
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK1_STACK_SIZE_BYTES 512

extern 	uint32_t 						task1_stack[];
extern 	os_event_handler_t 	irq_emulator_event;

void* task1 (void* a);
void* hook1 (void* p);

#ifdef __cplusplus
}
#endif


#endif
