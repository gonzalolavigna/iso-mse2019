#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task_debounce.h"


uint32_t task_debounce_stack[TASK_DEBOUNCE_STACK_SIZE_BYTES/4];

//Con este evento avisamos que tenemos algun evento de alguna tecla
os_event_handler_t 	tecla_event;

os_mutex_handler_t  mutex_tecla;


//Uso estas teclas porque las consecutivas por el temaño de los dedos se me complica
gpioMap_t teclas[] = {TEC2,TEC4};
#define TECLAS_VALIDAS sizeof(teclas)/sizeof(gpioMap_t)
debounce_data_t tecla_array[TECLAS_VALIDAS];

void debounce_init(void){
	uint32_t i;
	for(i=0;i<TECLAS_VALIDAS;i++){
		tecla_array[i].state = BUTTON_UP;
		tecla_array[i].tecla = teclas[i];
		tecla_array[i].tecla_falling_event = FALSE;
		tecla_array[i].tecla_rising_event  = FALSE;
		tecla_array[i].tecla_falling_tick  = 0;
		tecla_array[i].tecla_rising_tick 	 = 0;
	}
}

void* task_debounce (void* a){
	//Inicializamos la FSM de las teclas
	uint32_t i;
	bool_t 	 hit = FALSE;
	debounce_init();

	while(1){
		//Hacemos una espera inicial de 10ms en la tarea
		//Al principio no tenemos ningun evento que informar
		//Este codigo detecta un evento de tecla que se levanta
		hit = FALSE;
		//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
		os_mutex_lock(mutex_tecla);
		for(i=0;i<TECLAS_VALIDAS;i++){
			switch(tecla_array[i].state){
			case BUTTON_UP:
				tecla_array[i].tecla_falling_event = FALSE;
				tecla_array[i].tecla_rising_event  = FALSE;
					if(!gpioRead(tecla_array[i].tecla)){
						tecla_array[i].state = BUTTON_FALLING;
					}
				break;
			case BUTTON_FALLING:
					if(!gpioRead(tecla_array[i].tecla)){
						//Esto quiere decir que el tiempo del debouncer fue superado con exito
						tecla_array[i].state = BUTTON_DOWN;
						tecla_array[i].tecla_falling_event = TRUE;
						tecla_array[i].tecla_falling_tick  = os_get_ticks_running();
						//Si tenga un hit TRUE liberamos y seteamos el evento.
						hit = TRUE;
					}
					else {
						//Vuelvo a estar en BUTTON_UP
						tecla_array[i].state = BUTTON_UP;
					}
				break;
			case BUTTON_DOWN:
				tecla_array[i].tecla_falling_event = FALSE;
				tecla_array[i].tecla_rising_event  = FALSE;
				if(gpioRead(tecla_array[i].tecla))
					 tecla_array[i].state = BUTTON_RAISING;
				break;
			case BUTTON_RAISING:
					if(gpioRead(tecla_array[i].tecla)){
						tecla_array[i].state = BUTTON_UP;
						tecla_array[i].tecla_rising_event = TRUE;
						tecla_array[i].tecla_rising_tick 	= os_get_ticks_running();
						//Si tenga un hit TRUE liberamos y seteamos el evento.
						hit = TRUE;
					}
				break;
			default:
				break;
			}
		}
		//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
		os_mutex_unlock(mutex_tecla);
		//SI alguna tecla se solto ponemos el evento que se solto
		//una tecla
		if(hit == TRUE) {
			os_event_set(tecla_event);
		}
		//Esto lo ponemos en el delay con 10 ms para hacer el .
		os_task_delay(10);
	}
}


//Esto hace una copia rapida para hacer las cosas
void copy_tecla_array(debounce_data_t * dst ){
	uint32_t i;
	//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
	os_mutex_lock(mutex_tecla);
	//Esto hace una copia bastante rapida
	for(i=0;i<TECLAS_VALIDAS;i++){
		dst[i].state 			 						= tecla_array[i].state;
		dst[i].tecla 			 						= tecla_array[i].tecla;
		dst[i].tecla_falling_event 		= tecla_array[i].tecla_falling_event;
		dst[i].tecla_rising_event 		= tecla_array[i].tecla_rising_event;
		dst[i].tecla_falling_tick 		= tecla_array[i].tecla_falling_tick;
		dst[i].tecla_rising_tick  		= tecla_array[i].tecla_rising_tick;
	}
	//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
	os_mutex_unlock(mutex_tecla);
}

void* task_debounce_hook (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

