#ifndef __TASK3__
#define __TASK3__

#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TASK3_STACK_SIZE_BYTES 512

extern 	uint32_t 						task3_stack[];
extern 	os_event_handler_t 	tecla_event;

void* task3 (void* a);
void* hook3 (void* p);


#ifdef __cplusplus
}
#endif

#endif
