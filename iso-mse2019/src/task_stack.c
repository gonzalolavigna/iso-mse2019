#include <stdint.h>
#include "string.h"
#include "sapi.h"
#include "task_stack.h"


/*Funcion que inicializa un stack para guardar tareas*/
//TODO:Me gustaria que tuviera metodos push back push pop pop back pop front
void	 task_stack_init 			(task_stack_t *	task_stack){
	task_stack->stack_initializated = TRUE;
	task_stack->read_pointer 				= 0;
	task_stack->write_pointer 			= 0;
	task_stack->size								= 0;
}
/*Remueve un indice de tarea desde el frente de la cola usando el criterio FIFO*/
bool_t task_stack_pop 			(task_stack_t * task_stack,uint32_t * data ){
	if(task_stack->read_pointer == task_stack->write_pointer){
		return FALSE;
	}
	(*data) = task_stack->data[task_stack->read_pointer];
	task_stack->size--;
	task_stack->read_pointer = ((task_stack->read_pointer+1)%STACK_SIZE);
	return TRUE;
}
/*Agrega un item al final de la cola, uno le pasa el indice de la tarea*/
bool_t task_stack_push			(task_stack_t * task_stack,uint32_t   data ){
	if(task_stack->size >= STACK_SIZE){
		return FALSE;
	}
	task_stack->data[task_stack->write_pointer] = data;
	task_stack->size++;
	task_stack->write_pointer = ((task_stack->write_pointer+1)%STACK_SIZE);
	return TRUE;
}
/*Devulve si la cola esta vacia o entera*/
bool_t task_stack_is_empty 	(task_stack_t * task_stack){
	if(task_stack->size == 0){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

/*Devuelve si la cola esta llena*/
bool_t task_stack_is_full  	(task_stack_t * task_stack){
	if(task_stack->size == 0){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

/*No se usa aun y no esta probada pero no sirve para sacar un item de la cola*/
/*Hace uso de pop and push para devolver los items a su lugar original*/
bool_t tack_stack_remove_item(task_stack_t * task_stack,uint32_t item){
	uint32_t i;
	uint32_t data;
	bool_t 	 item_hit = FALSE;
	if(task_stack_is_empty(task_stack) == FALSE){
		for(i=0;i<task_stack->size;i++){
			if(task_stack_pop(task_stack,&data) == TRUE){
				//Todos los items que sean distintos los vuelvo a poner en la cola
				if(data != item)
					task_stack_push(task_stack,data);
				else
					item_hit = TRUE;
			}
			else {
				//Hay algun problema ya que el item deberia estar
				item_hit = FALSE;
			}
		}
	}
	else{
		item_hit = FALSE;
	}
	return item_hit;
}
/*Nos devuelve el tamaño de la cola*/
uint32_t task_stack_get_size 		(task_stack_t * task_stack){
	return task_stack->size;
}

