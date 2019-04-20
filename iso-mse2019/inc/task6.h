#ifndef __TASK6__
#define __TASK6__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi_circularBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK6_STACK_SIZE_BYTES 2048

extern	uint32_t 					task6_stack[];
//Hacemos un extern para que otros puedan usar el buffer circular de la SAPI
extern  uint8_t 	 				uart_buffer_array[];
extern 	circularBuffer_t 	uart_buffer_sapi;

void init_uart_circular_buffer(void);
void* task6										(void* a);
void* hook6										(void* p);


#ifdef __cplusplus
}
#endif

#endif
