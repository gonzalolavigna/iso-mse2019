#ifndef __OS__
#define __OS__

#include <stdint.h>
#include "sapi.h"
#include "task_stack.h"

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
	HIGH_PRIORITY 		= 0,
	MEDIUM_PRIORITY,
	LOW_PRIORITY,
	IDLE_PRIORITY 		= 0xFFFFFFFF,
} task_priority_t;

/*Datos para poder tener la informacion de la tarea actual.*/
typedef struct  {
	task_state_t 		state;
	uint32_t				stack_pointer;
	uint32_t				task_index;
	uint32_t				reamaining_ticks;
	task_priority_t	priority;
	uint32_t*				initial_stack_pointer;
	uint32_t 				stack_size_bytes;
}task_context_t;


/*==================[declaraciones de funciones externas]====================*/
bool_t 	os_init 				(	void);
bool_t 	os_task_create	(	uint32_t stack[],
													uint32_t stack_size_bytes,
													task_type_f entry_point,
													task_priority_t priority,
													void * arg );
bool_t 	os_queue_init		(	void);
void 		do_scheduler		(	void);
void 		os_error_hook		(	void);


/*==================[declaracion datos a utilizarse externamente=============*/
extern task_context_t task_list[];
extern uint32_t 			running_task_index;
extern uint32_t 			task_count;
extern task_stack_t 	priority_queue[];

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
