#ifndef __TASK6__
#define __TASK6__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK6_STACK_SIZE_BYTES 512

extern	uint32_t 					task6_stack[];
extern 	circularBuffer_t 	uart_buffer;

void init_uart_circular_buffer(void);
void* task6										(void* a);
void* hook6										(void* p);

#endif
