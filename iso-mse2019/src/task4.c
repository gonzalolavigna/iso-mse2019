#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task3.h"
#include "task4.h"
#include "task5.h"


uint32_t task4_stack[TASK4_STACK_SIZE_BYTES/4];


gpioMap_t teclas[] = {TEC1,TEC2,TEC3,TEC4};
#define TECLAS_VALIDAS sizeof(teclas)/sizeof(gpioMap_t)
debounce_data_t tecla_array[TECLAS_VALIDAS];

void debounce_init(void){
	uint32_t i;
	for(i=0;i<TECLAS_VALIDAS;i++){
		tecla_array[i].state = BUTTON_UP;
		tecla_array[i].tecla = teclas[i];
		tecla_array[i].tecla_liberada_event = FALSE;
		tecla_array[i].tecla_presionada_event = FALSE;
		tecla_array[i].ticks_presionada = 0;
		tecla_array[i].primer_tick = 0;
	}
}



void* task4 (void* a){
	//Inicializamos la FSM de las teclas
	uint32_t i;
	bool_t 	 hit = FALSE;
	debounce_init();

	while(1){
		//Hacemos una espera inicial de 10ms en la tarea
		//Al principio no tenemos ningun evento que informar
		//Este codigo detecta un evento de tecla que se levanta
		hit = FALSE;
		for(i=0;i<TECLAS_VALIDAS;i++){
			switch(tecla_array[i].state){
			case BUTTON_UP:
					tecla_array[i].tecla_liberada_event = FALSE;
					if(!gpioRead(tecla_array[i].tecla)){
						tecla_array[i].state = BUTTON_FALLING;
						tecla_array[i].ticks_presionada = 0;
						tecla_array[i].primer_tick = 0;
					}
				break;
			case BUTTON_FALLING:
					if(!gpioRead(tecla_array[i].tecla)){
						//Esto quiere decir que el tiempo del debouncer fue superado con exito
						tecla_array[i].state = BUTTON_DOWN;
						tecla_array[i].primer_tick = os_get_ticks_running();
					}
					else {
						//Vuelvo a estar en BUTTON_UP
						tecla_array[i].state = BUTTON_UP;
					}
				break;
			case BUTTON_DOWN:
				 if(gpioRead(tecla_array[i].tecla))
					 tecla_array[i].state = BUTTON_RAISING;
				break;
			case BUTTON_RAISING:
					if(gpioRead(tecla_array[i].tecla)){
						tecla_array[i].state = BUTTON_UP;
						tecla_array[i].ticks_presionada = os_get_ticks_running() - tecla_array[i].primer_tick;
						tecla_array[i].tecla_liberada_event = TRUE;
						//Si tenga un hit TRUE liberamos y seteamos el evento.
						hit = TRUE;
					}
				break;
			default:
				break;
			}
		}
		//SI alguna tecla se solto ponemos el evento que se solto
		//una tecla
		if(hit == TRUE) {
			os_event_set(tecla_event);
			if(tecla_array[TECLA_1_INDEX].tecla_liberada_event == TRUE){
				/*Informamos que solamente esta la tecla 1 event*/
				os_event_set(tecla_1_event);
			}
		}
		//Esto lo ponemos en el delay con 10.
		os_task_delay(10);
	}
}


//Esto hace una copia rapida para hacer las cosas
void copy_tecla_array(debounce_data_t * dst ){
	uint32_t i;
	//Esto hace una copia bastante rapida
	for(i=0;i<TECLAS_VALIDAS;i++){
		dst[i].primer_tick 						= tecla_array[i].primer_tick;
		dst[i].state 			 						= tecla_array[i].state;
		dst[i].tecla 			 						= tecla_array[i].tecla;
		dst[i].tecla_liberada_event 	= tecla_array[i].tecla_liberada_event;
		dst[i].tecla_presionada_event = tecla_array[i].tecla_presionada_event;
		dst[i].ticks_presionada 			= tecla_array[i].ticks_presionada;
	}
}

void* hook4 (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

