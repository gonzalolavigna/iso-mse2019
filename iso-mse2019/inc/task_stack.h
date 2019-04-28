#ifndef __TASK_STACK__
#define __TASK_STACK__

#include <stdint.h>
#include "sapi.h"

#ifdef __cplusplus
extern "C" {
#endif

//Mismo numero que la cantidad de tarea, podria dismunuirse para encotrar un optimo.
///TODO:Despues de haber pasado por muchas cosa es preferible hacer una lista enlazada. Quedara para una proxima implementacion.
//Lo bueno que de esta manera es relativamente rapido hacer el round robin.
#define STACK_SIZE 10

typedef struct {
	uint32_t data[STACK_SIZE];
	uint32_t read_pointer;
	uint32_t write_pointer;
	uint32_t size;
	bool_t	 stack_initializated;
} task_stack_t;


void	 		task_stack_init 				(task_stack_t *	task_stack);
bool_t 		task_stack_pop 					(task_stack_t * task_stack,uint32_t * data );
bool_t 		task_stack_push					(task_stack_t * task_stack,uint32_t   data );
bool_t	 	task_stack_is_empty 		(task_stack_t * task_stack);
bool_t		task_stack_is_full  		(task_stack_t * task_stack);
uint32_t	task_stack_get_size 		(task_stack_t * task_stack);
bool_t 		tack_stack_remove_item	(task_stack_t * task_stack,uint32_t item);

#ifdef __cplusplus
}
#endif

#endif
