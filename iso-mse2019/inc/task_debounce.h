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

//Tamaño0 del stack de la tarea correspondiente al debounce de la tecla
#define TASK_DEBOUNCE_STACK_SIZE_BYTES 512
//La cola de mensajes del OS es de 10 eventos lo cual es un monton, creo que con 3 alcanza y sobra.
#define OS_QUEUE_TECLA_IRQ_SIZE 10
//El debounce que implementamos es de 10ms. //TODO:Pasar a un esquema en el futuro de ms a ticks
#define DEBOUNCE_TICKS 								 10
//Desde afuera los indices de las teclas se entienden ascendentemente aunque usamos efectivamente las teclas 2 y 4.
#define TECLA_1_INDEX 0
#define TECLA_2_INDEX 1

//Para poder ser llamado en el main e inicializar el stack de la tarea
extern uint32_t task_debounce_stack[];
//Evento para manejar cuando sucede un flanco valida y ser atendido por la FSM que mide los tiempos.
extern os_event_handler_t 	tecla_event;
//Esto sirve para proteger los datos de las teclas mientra accese la FSM que mide los tiempos
extern os_mutex_handler_t  mutex_tecla;

//Posibles estados en los que puede estar una tecla, este lo usamos para validar flancos.
//Con la implementacion actual no es necesario BUTTON_FALLING y BUTTON_RAISIN
typedef enum {
	BUTTON_UP = 0,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING,
}fsmDebounce_t;

//Para indicar si un flanco es ascendente o descendente
typedef enum {
	RISING_EDGE = 0,
	FALLING_EDGE,
} edge_t;

//Esta es la estructura que contiene la informacion de una tecla, es toda la informacion que necesita
//la FSM que mide los tiempos para decidir que acción realizar con los print y los leds.
typedef struct {
	gpioMap_t 		tecla;
	fsmDebounce_t state;
	bool_t 				tecla_rising_event;
	bool_t 				tecla_falling_event;
	uint32_t 			tecla_rising_tick;
	uint32_t 			tecla_falling_tick;
}debounce_data_t;

//A la tarea que realiza el debouncing las IRQ le pasan esta informacion para que la misma realice el
//correspondiente filtrado.
typedef struct {
	uint32_t  		tecla_index;
	edge_t				edge;
	uint32_t 			tick;
}tecla_irq_data_t;

//Funciones para ser usadas por el OS
void* task_debounce 			(void* a);
void* task_debounce_hook	(void* p);

//Funciones para ser usadas por otras tareas
//Inicialzamos la cola de mensaje del OS
bool_t cola_teclas_init(void);
//Funcion para que la tarea FSM tome los datos de todas las teclas
void 	copy_tecla_array  	(debounce_data_t * dst );

#ifdef __cplusplus
}
#endif

#endif
