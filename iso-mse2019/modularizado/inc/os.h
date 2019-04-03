#ifndef __OS__
#define __OS__

#include <stdint.h>
#include "sapi.h"

/*==================[inclusiones]============================================*/
/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*Tipos de la tareas que acepta es OS*/
typedef void * (*task_type_f)(void *);

/*Las tareas pueden estar en 3 estados RUNNING - READY - SLEEPING*/
typedef enum {
	TASK_RUNNING = 0,
	TASK_READY,
	TASK_SLEEPING
} task_state_t ;

typedef enum {
	OS_INIT = 0,	 			//Este es el estado en el cual arranca el S.O
	OS_RUNNING_TASK, 	 //Una vez que hace el primer switch arrancamos en modo nominal
	OS_RUNNING_IDLE,
} os_state_t;

typedef enum {
	IDLE_PRIORITY 	= 0,
	LOW_PRIORITY,
	MEDIUM_PRIORITY,
	HIGH_PRIORITY
} task_priority_t;

/*Datos para poder tener la informacion de la tarea actual.*/
typedef struct  {
	task_state_t 		state;
	uint32_t*				stack_pointer;
	uint32_t 				task_index;
	uint32_t 				reamaining_ticks;
	task_priority_t	priority;
	uint32_t*				initial_stack_pointer;
	uint32_t 				stack_size_bytes;
}task_context_t;


/*==================[declaraciones de funciones externas]====================*/
bool_t os_init (void);

bool_t os_task_create	(	uint32_t stack[],uint32_t stack_size_bytes,task_type_f entry_point,
							task_priority_t priority, void * arg );

bool_t os_task_delay	(uint32_t ticks);


/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
