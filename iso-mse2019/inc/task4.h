#ifndef __TASK4__
#define __TASK4__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK4_STACK_SIZE_BYTES 512

extern uint32_t task4_stack[];

typedef enum {
	BUTTON_UP = 0,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING,
}fsmDebounce_t;


typedef struct {
	gpioMap_t 		tacla;
	fsmDebounce_t state;
	bool_t 				tecla_liberada;
	uint32_t 			ticks_presionada;
}debounce_data_t;


void* task4 (void* a);
void* hook4 (void* p);


#ifdef __cplusplus
}
#endif

#endif
