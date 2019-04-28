#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task_debounce.h"
#include "task_print.h"

//Estos datos se comparten externamente
uint32_t 	task_print_stack[TASK_PRINT_STACK_SIZE_BYTES/4];

//Los datos de medicion
edge_measure_data_t edge_measure_data;

//Copia temporal de los arreglos
static debounce_data_t tecla_array_copy[2];

//Eventos del SO para poder prender los distintos leds
os_event_handler_t event_led_green;
os_event_handler_t event_led_red;
os_event_handler_t event_led_yellow;
os_event_handler_t event_led_blue;

//Mutex para proteger la seccion correspondiente a los led
os_mutex_handler_t mutex_led;


void init_edge_detector_fsm (void){
	edge_measure_data.falling_edge_tick_tec_1 = 0;
	edge_measure_data.falling_edge_tick_tec_2 = 0;
	edge_measure_data.rising_edge_tick_tec_1  = 0;
	edge_measure_data.rising_edge_tick_tec_2  = 0;
	edge_measure_data.led_event 							= NOTHING;
	edge_measure_data.state 									= WAIT_FIRST_FALLING_EDGE;
	edge_measure_data.time_1 									= 0;
	edge_measure_data.time_2 									= 0;
}

bool_t edge_dector_fsm (void){
	bool_t hit = FALSE;
	switch(edge_measure_data.state){
	case WAIT_FIRST_FALLING_EDGE:
		//printf("WAIT_FIRST_FALLING_EDGE\r\n");
		//En este estado volvemos a inicializar la FSM
		init_edge_detector_fsm();
		//Si por una de esa casualidades las dos teclas se detectan al mismo tiempo, nos vamos a esperar rising edge
		if((tecla_array_copy[TECLA_1_INDEX].tecla_falling_event == TRUE) && (tecla_array_copy[TECLA_2_INDEX].tecla_falling_event == TRUE) ){
			edge_measure_data.time_1 = 0;
			edge_measure_data.falling_edge_tick_tec_1 = tecla_array_copy[TECLA_1_INDEX].tecla_falling_tick;
			edge_measure_data.falling_edge_tick_tec_2 = tecla_array_copy[TECLA_2_INDEX].tecla_falling_tick;
			edge_measure_data.state  = WAIT_FIRST_RISING_EDGE;
		} else if (tecla_array_copy[TECLA_1_INDEX].tecla_falling_event == TRUE) {
			edge_measure_data.falling_edge_tick_tec_1 = tecla_array_copy[TECLA_1_INDEX].tecla_falling_tick;
			edge_measure_data.state  									= WAIT_FALLING_EDGE_TEC_2;
		} else if (tecla_array_copy[TECLA_2_INDEX].tecla_falling_event == TRUE) {
			edge_measure_data.falling_edge_tick_tec_2 = tecla_array_copy[TECLA_2_INDEX].tecla_falling_tick;
			edge_measure_data.state  									= WAIT_FALLING_EDGE_TEC_1;
		} else {
			//Espero en este estado
			edge_measure_data.state  = WAIT_FIRST_FALLING_EDGE;
		}
		break;
	case WAIT_FALLING_EDGE_TEC_1:
		//printf("WAIT_FALLING_EDGE_TEC_1\r\n");
		//Si estoy aca significa que pulsaron la tecla 2 y si hay un flanco ascendente ya no hay nada que medir
		if(tecla_array_copy[TECLA_2_INDEX].tecla_rising_event == TRUE){
			edge_measure_data.state = WAIT_FIRST_FALLING_EDGE;
		}	else if(tecla_array_copy[TECLA_1_INDEX].tecla_falling_event == TRUE){
			edge_measure_data.falling_edge_tick_tec_1 = tecla_array_copy[TECLA_1_INDEX].tecla_falling_tick;
			//Este el time_1 que ya lo medi
			edge_measure_data.time_1 = edge_measure_data.falling_edge_tick_tec_1 - edge_measure_data.falling_edge_tick_tec_2;
			edge_measure_data.state  = WAIT_FIRST_RISING_EDGE;
		} else {
			//Si no hay nada que me interese me quedo aca
			edge_measure_data.state = WAIT_FALLING_EDGE_TEC_1;
		}
		break;
	case WAIT_FALLING_EDGE_TEC_2:
		//printf("WAIT_FALLING_EDGE_TEC_2\r\n");
		//Si estoy aca significa que pulsaron la tecla 1 y si hay un flanco ascendente ya no hay nada que medir
		if(tecla_array_copy[TECLA_1_INDEX].tecla_rising_event == TRUE){
			edge_measure_data.state = WAIT_FIRST_FALLING_EDGE;
		}	else if(tecla_array_copy[TECLA_2_INDEX].tecla_falling_event == TRUE){
			edge_measure_data.falling_edge_tick_tec_2 = tecla_array_copy[TECLA_2_INDEX].tecla_falling_tick;
			//Este el time_1 que ya lo medi
			edge_measure_data.time_1 = edge_measure_data.falling_edge_tick_tec_2 - edge_measure_data.falling_edge_tick_tec_1;
			edge_measure_data.state  = WAIT_FIRST_RISING_EDGE;
		} else {
			//Si no hay nada que me interese me quedo aca
			edge_measure_data.state = WAIT_FALLING_EDGE_TEC_2;
		}
		break;
	case WAIT_FIRST_RISING_EDGE:
		//printf("WAIT_FIRST_RISING_EDGE\r\n");
		//Estamos esperando un ascendente esto no deberia suceder
		if((tecla_array_copy[TECLA_1_INDEX].tecla_falling_event == TRUE) || (tecla_array_copy[TECLA_2_INDEX].tecla_falling_event == TRUE) ){
			edge_measure_data.state = WAIT_FIRST_FALLING_EDGE;
		//Esto significa que cayeron los en el mismo tick lo cual los detecto como un evento
		} else if((tecla_array_copy[TECLA_1_INDEX].tecla_rising_event == TRUE) && (tecla_array_copy[TECLA_2_INDEX].tecla_rising_event == TRUE)){
			edge_measure_data.rising_edge_tick_tec_1  = tecla_array_copy[TECLA_1_INDEX].tecla_rising_tick;
			edge_measure_data.rising_edge_tick_tec_2  = tecla_array_copy[TECLA_2_INDEX].tecla_rising_tick;
			//Ya esta termine me voy al principio
			edge_measure_data.state  = WAIT_FIRST_FALLING_EDGE;
			//No hay diferencia de flancos ascendentes
			edge_measure_data.time_2 = 0;
			hit = TRUE;
		}	else if(tecla_array_copy[TECLA_1_INDEX].tecla_rising_event == TRUE){
			edge_measure_data.rising_edge_tick_tec_1 = tecla_array_copy[TECLA_1_INDEX].tecla_rising_tick;
			edge_measure_data.state  									= WAIT_RISING_EDGE_TEC_2;
		} else if(tecla_array_copy[TECLA_2_INDEX].tecla_rising_event == TRUE) {
			edge_measure_data.rising_edge_tick_tec_2 = tecla_array_copy[TECLA_2_INDEX].tecla_rising_tick;
			edge_measure_data.state  									= WAIT_RISING_EDGE_TEC_1;
		} else {
			edge_measure_data.state = WAIT_FIRST_RISING_EDGE;
		}
		break;
	//Quiere decir que se viene de la tecla 2
	case WAIT_RISING_EDGE_TEC_1:
		//printf("WAIT_RISING_EDGE_TEC_1\r\n");
		//Si estoy aca significa que pulsaron la tecla 2 antes y si hay un flanco descendente quiere decir que la tecla
		//La volvieron a pulsar de vuelta asi que perdemos la referencia de la medicion.
		//Aca se pueden usar otros criterios
		if(tecla_array_copy[TECLA_2_INDEX].tecla_falling_event == TRUE){
			edge_measure_data.state = WAIT_FIRST_FALLING_EDGE;
		}	else if(tecla_array_copy[TECLA_1_INDEX].tecla_rising_event== TRUE){
			edge_measure_data.rising_edge_tick_tec_1 = tecla_array_copy[TECLA_1_INDEX].tecla_rising_tick;
			//Este el time_1 que ya lo medi
			edge_measure_data.time_2 = edge_measure_data.rising_edge_tick_tec_1 - edge_measure_data.rising_edge_tick_tec_2;
			edge_measure_data.state  = WAIT_FIRST_FALLING_EDGE;
			hit = TRUE;
		} else {
			//Si no hay nada que me interese me quedo aca
			edge_measure_data.state = WAIT_RISING_EDGE_TEC_1;
		}
		break;
	case WAIT_RISING_EDGE_TEC_2:
		//printf("WAIT_RISING_EDGE_TEC_2\r\n");
		//Si estoy aca significa que pulsaron la tecla 1 antes y si hay un flanco descendente quiere decir que la tecla
		//La volvieron a pulsar de vuelta asi que perdemos la referencia de la medicion.
		//Aca se pueden usar otros criterios
		if(tecla_array_copy[TECLA_1_INDEX].tecla_falling_event == TRUE){
			edge_measure_data.state = WAIT_FIRST_FALLING_EDGE;
		}	else if(tecla_array_copy[TECLA_2_INDEX].tecla_rising_event== TRUE){
			edge_measure_data.rising_edge_tick_tec_2 = tecla_array_copy[TECLA_2_INDEX].tecla_rising_tick;
			//Este el time_1 que ya lo medi
			edge_measure_data.time_2 = edge_measure_data.rising_edge_tick_tec_2 - edge_measure_data.rising_edge_tick_tec_1;
			edge_measure_data.state  = WAIT_FIRST_FALLING_EDGE;
			hit = TRUE;
		} else {
			//Si no hay nada que me interese me quedo aca
			edge_measure_data.state = WAIT_RISING_EDGE_TEC_2;
		}
		break;
	default:
		uartWriteString(UART_USB,"Error en FSM\r\n");
		break;
	}
	return hit;
}


void* task_print (void* a){
	bool_t hit = FALSE;
	uint32_t i;
	uartWriteString(UART_USB,"ISO-MSE-2019:Final\r\n");
	init_edge_detector_fsm();
	while(1){
		if(os_event_wait(tecla_event) == TRUE){
			copy_tecla_array(tecla_array_copy);

			//En esta seccion se sobreescriben los datos de la estructura de control
			os_mutex_lock(mutex_led);
			hit = edge_dector_fsm();
			os_mutex_unlock(mutex_led);
			if(hit == TRUE){
				printf("ALGO QUE INFORMAR\r\n");
				if(edge_measure_data.falling_edge_tick_tec_2 > edge_measure_data.falling_edge_tick_tec_1){
					if(edge_measure_data.rising_edge_tick_tec_2 > edge_measure_data.rising_edge_tick_tec_1){
						//LED VERDE
						printf("LED VERDE ENCENDIDO:\r\n");
						printf("\tTIEMPO ENCENDIDO:%d ms\r\n",edge_measure_data.time_1 + edge_measure_data.time_2 );
						printf("\tTIEMPO ENTRE FLANCOS DESCENDENTES:%d ms\r\n",edge_measure_data.time_1);
						printf("\tTIEMPO ENTRE FLANCOS ASCENDENTES:%d ms\r\n",edge_measure_data.time_2);
						edge_measure_data.led_event = GREEN_EVENT;
						os_event_set(event_led_green);
					}else{
						//LED ROJO
						printf("LED ROJO ENCENDIDO:\r\n");
						printf("\tTIEMPO ENCENDIDO:%d ms\r\n",edge_measure_data.time_1 + edge_measure_data.time_2 );
						printf("\tTIEMPO ENTRE FLANCOS DESCENDENTES:%d ms\r\n",edge_measure_data.time_1);
						printf("\tTIEMPO ENTRE FLANCOS ASCENDENTES:%d ms\r\n",edge_measure_data.time_2);
						edge_measure_data.led_event = RED_EVENT;
						os_event_set(event_led_red);
					}
				}else {
					if(edge_measure_data.rising_edge_tick_tec_2 > edge_measure_data.rising_edge_tick_tec_1){
						//LED AMARILLO
						printf("LED AMARILLO ENCENDIDO:\r\n");
						printf("\tTIEMPO ENCENDIDO:%d ms\r\n",edge_measure_data.time_1 + edge_measure_data.time_2 );
						printf("\tTIEMPO ENTRE FLANCOS DESCENDENTES:%d ms\r\n",edge_measure_data.time_1);
						printf("\tTIEMPO ENTRE FLANCOS ASCENDENTES:%d ms\r\n",edge_measure_data.time_2);
						edge_measure_data.led_event = YELLOW_EVENT;
						os_event_set(event_led_yellow);
					}else{
						//LED AZUL
						printf("LED AZUL ENCENDIDO:\r\n");
						printf("\tTIEMPO ENCENDIDO:%d ms\r\n",edge_measure_data.time_1 + edge_measure_data.time_2 );
						printf("\tTIEMPO ENTRE FLANCOS DESCENDENTES:%d ms\r\n",edge_measure_data.time_1);
						printf("\tTIEMPO ENTRE FLANCOS ASCENDENTES:%d ms\r\n",edge_measure_data.time_2);
						edge_measure_data.led_event = BLUE_EVENT;
						os_event_set(event_led_blue);
					}
				}
			}
		}else {
			uartWriteString(UART_USB,"TASK_PRINT:EVENT WAIT\r\n");
		}
	}
}
void* hook_print (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

uint32_t get_led_delay_time (void){
	uint32_t temp;
	//Agarro el tiempo que me dan para retrasar a los led
	os_mutex_lock(mutex_led);
	temp = edge_measure_data.time_1 + edge_measure_data.time_2;
	os_mutex_unlock(mutex_led);
	return temp;
}


//printf("EVENTO RECIBIDO\r\n");
//for(i=0;i<2;i++){
	//if(tecla_array_copy[i].tecla_falling_event == TRUE){
	//	printf("TASK PRINT:TECLA %d FLANCO DESCENDENTE\r\n",i+1);
	//}
	//if(tecla_array_copy[i].tecla_rising_event == TRUE){
	//printf("TASK PRINT:TECLA %d FLANCO ASCENDENTE\r\n",i+1);
	//}
//}
