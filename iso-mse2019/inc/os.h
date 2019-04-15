#ifndef __OS__
#define __OS__

#include <stdint.h>
#include "sapi.h"
#include "task_stack.h"

/*==================[inclusiones]============================================*/
//Numero maximo de las tareas permitidas por nuestro OS
#define MAX_TASK_COUNT 10
//Numero maximo de los eventos permitidas por nuestro OS
#define MAX_EVENT_COUNT 5

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*Tipos de la tareas que acepta es OS*/
/*En resumen reciben parametros por void * y si salen lo devuelve por void**/
/*Por el momento no esta hecho el return pero por lo menos no nos ponemos la soga
 * al cuello*/
typedef void * (*task_type_f)(void *);

/*Las tareas pueden estar en 3 estados RUNNING - READY - SLEEPING*/
typedef enum {
	TASK_RUNNING = 0,
	TASK_READY,
	TASK_SLEEPING
} task_state_t ;

/*El SO arranca en OS_INIT y lo usa para hacer el primer cambio de contexto */
/*Cuando viene de correr una tarea el case que se ejecuta es el OS es OS_RUNNING_TASK*/
/*Cuando viene de viene de correr la tarea IDLE se ejecuta es OS_RUNNING_IDLE*/
typedef enum {
	OS_INIT = 0,
	OS_RUNNING_TASK,
	OS_RUNNING_IDLE,
} os_state_t;

/*Planteamos un SO con 3 prioridades solamente, la idea es que pueden agregarse mas
 * pero de esta manera vemos como se hace el scheduling*/
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
	bool_t					event_waiting;
}task_context_t;


/*==================[declaraciones de funciones externas]====================*/
/*Inicializa el SO, tambien el Sistick, la pendsv y configura el stack de la tarea IDLE*/
bool_t 	os_init 				(	void);
/*Funcion para crear una tarea para el sistema operativo IMPORTANTE: El stack lo debe crear
 * cada tarea el espacio en memoria para inicializarlo*/
bool_t 	os_task_create	(	uint32_t stack[],
													uint32_t stack_size_bytes,
													task_type_f entry_point,
													task_priority_t priority,
													void * arg );
/*Por cuestiones constructivas las colas de prioridades hay que construirlas antes de usar
 * el SO, esto podria hacerswe en OS_INIT TODO:Pasarlos OS_INIT */
bool_t 	os_queue_init		(	void);
/*Esta funcion es equivalente a un YIELD y la idea es con esta activar la pendsv y forzar
 * un scheduling y posterior cambio de contexto*/
void 		do_scheduler		(	void);
/*ALgunas cuestiones mientras ejecutamos el OS pueden llevar a errores los constrolamos yendo
 * a os_error_hook para poder encontrar mas facil los errores con el debugger*/
void 		os_error_hook		(	void);

void* 	idle_task(void * arg);

/*==================[declaracion datos a utilizarse externamente=============*/
/*Esta es la lista de todas las tareas, claramente la necesitamos para pasar tareas
 * a sleeping, manejar los delays y tambien los eventos de semaforos*/
extern task_context_t task_list[];
/*Esta es la tarea que esta corriendo ahora, sirve para bloquear tareas cuando llaman
 * a delay o un event_waint*/
extern uint32_t 			running_task_index;
/*En algunas funciones se hace un control para no tener un index mayor a las tareas
 * corridas ya que haria colapsar el OS TODO:Crear un estructura con los datosw del OS*/
extern uint32_t 			task_count;
/*Cuando hay que reactivar un tarea se hace un push en su cola de prioridad, esto se hace
 * cuando termina un delay y tambien cuando un evento fue enviado*/
extern task_stack_t 	priority_queue[];

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
