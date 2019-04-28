#ifndef __TASK_DEBOUNCE__
#define __TASK_DEBOUNCE__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_DEBOUNCE_STACK_SIZE_BYTES 512

extern uint32_t task_debounce_stack[];
//Evento para manejar la tarea para indicar que algun evento nuevo
extern os_event_handler_t 	tecla_event;
extern os_event_handler_t   tecla_irq_event;

//Esto sirve para proteger los datos de las teclas
extern os_mutex_handler_t  mutex_tecla;

typedef enum {
	BUTTON_UP = 0,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING,
}fsmDebounce_t;

typedef enum {
	RISING_EDGE = 0,
	FALLING_EDGE,
} edge_t;


#define TECLA_1_INDEX 0
#define TECLA_2_INDEX 1

typedef struct {
	gpioMap_t 		tecla;
	fsmDebounce_t state;
	bool_t 				tecla_rising_event;
	bool_t 				tecla_falling_event;
	uint32_t 			tecla_rising_tick;
	uint32_t 			tecla_falling_tick;
}debounce_data_t;


typedef struct {
	uint32_t  		tecla_index;
	edge_t				edge;
	uint32_t 			tick;
}tecla_irq_data_t;


void* task_debounce 			(void* a);
void* task_debounce_hook	(void* p);

//Funciones de usuario
void 	debounce_init				(void);
void 	copy_tecla_array  	(debounce_data_t * dst );

//Inicializamos la IRQ de GPIO
void init_irq_gpio(void);


#ifdef __cplusplus
}
#endif

#endif
