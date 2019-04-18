#ifndef __TASK5__
#define __TASK5__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK5_STACK_SIZE_BYTES 512

extern	uint32_t 						task5_stack[];
extern 	os_event_handler_t 	tecla_1_event;

void* task5	(void* a);
void* hook5	(void* p);

#endif



