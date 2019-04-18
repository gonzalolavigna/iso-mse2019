#ifndef __TASK3__
#define __TASK3__

#include <stdint.h>
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK3_STACK_SIZE_BYTES 	512
#define BUFFER_RESULT_SIZE 			240

void* task3 (void* a);
void* hook3 (void* p);

//El stack se usa para inicializar la tarea
extern 	uint32_t 						task3_stack[];
extern 	os_event_handler_t 	result_ready;

#ifdef __cplusplus
}
#endif


#endif
