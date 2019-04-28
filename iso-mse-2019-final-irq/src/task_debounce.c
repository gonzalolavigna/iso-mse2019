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
os_event_handler_t  tecla_irq_event;

os_mutex_handler_t  mutex_tecla;


//Uso estas teclas porque las consecutivas por el temaño de los dedos se me complica
gpioMap_t teclas[] = {TEC2,TEC4};
#define TECLAS_VALIDAS sizeof(teclas)/sizeof(gpioMap_t)
debounce_data_t tecla_array[TECLAS_VALIDAS];

tecla_irq_data_t  tecla_irq_data;


void serve_gpio_irq(uint32_t tecla_index,edge_t edge);

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

/*
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
		os_task_delay(1);
	}
}
*/

void* task_debounce (void* a){
	bool_t 	 hit = FALSE;
	uint32_t i;
	debounce_init();
	while(1){
		os_event_wait(tecla_irq_event);
		hit = FALSE;
		//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
		os_mutex_lock(mutex_tecla);
		//Reseteo esta informacion de evento para pasarla a la FSM que decide la medicion.
		for(i=0;i<TECLAS_VALIDAS;i++){
			tecla_array[i].tecla_falling_event = FALSE;
			tecla_array[i].tecla_rising_event  = FALSE;
		}
		switch(tecla_array[tecla_irq_data.tecla_index].state){
		case BUTTON_UP:
			if(tecla_irq_data.edge == FALLING_EDGE){
				tecla_array[tecla_irq_data.tecla_index].tecla_falling_event = TRUE;
				tecla_array[tecla_irq_data.tecla_index].tecla_falling_tick  = tecla_irq_data.tick;
				tecla_array[tecla_irq_data.tecla_index].state 					    = BUTTON_DOWN;
				hit = TRUE;
			}
			break;
		case BUTTON_DOWN:
			if(tecla_irq_data.edge == RISING_EDGE){
				tecla_array[tecla_irq_data.tecla_index].tecla_rising_event  = TRUE;
				tecla_array[tecla_irq_data.tecla_index].tecla_rising_tick  =  tecla_irq_data.tick;
				tecla_array[tecla_irq_data.tecla_index].state 					    = BUTTON_UP;
				hit = TRUE;
			}
			break;
		default:
			break;

		}
		if(hit == TRUE){
			os_event_set(tecla_event);
		}
		//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
		os_mutex_unlock(mutex_tecla);
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



void init_irq_gpio(void){
	//En el canal de interrupcion 0
	//Flanco Descendente TEC 2
	Chip_SCU_GPIOIntPinSel(0,0,8);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(0));

	//En el canal de interrupcion 1
	//Flanco Ascendente TEC 2
	Chip_SCU_GPIOIntPinSel(1,0,8);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(1));

	//En el canal de interrupcion 2
	//Flanco Descendente TEC 4
	Chip_SCU_GPIOIntPinSel(2,1,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(2));

	//En el canal de interrupcion 3
	//Flanco Ascendente TEC 4
	Chip_SCU_GPIOIntPinSel(3,1,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(3));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(3));

	//Configuro prioridad de interrupciones
	//Le pongo la prioridad 1 mas que la pendsv
	NVIC_SetPriority(PIN_INT0_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT1_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT2_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT3_IRQn,(1 << __NVIC_PRIO_BITS) - 2);

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
  NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
  NVIC_ClearPendingIRQ( PIN_INT3_IRQn );

  //Habilito interrupciones
  NVIC_EnableIRQ( PIN_INT0_IRQn );
  NVIC_EnableIRQ( PIN_INT1_IRQn );
  NVIC_EnableIRQ( PIN_INT2_IRQn );
  NVIC_EnableIRQ( PIN_INT3_IRQn );
}

void GPIO0_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	serve_gpio_irq(TECLA_1_INDEX,FALLING_EDGE);
}

void GPIO1_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	serve_gpio_irq(TECLA_1_INDEX,RISING_EDGE);
}

void GPIO2_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	serve_gpio_irq(TECLA_2_INDEX,FALLING_EDGE);
}

void GPIO3_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	serve_gpio_irq(TECLA_2_INDEX,RISING_EDGE);
}

void serve_gpio_irq(uint32_t tecla_index,edge_t edge){
	tecla_irq_data.tecla_index = tecla_index;
	tecla_irq_data.edge = edge;
	tecla_irq_data.tick = os_get_ticks_running();
	os_event_set_from_irq(tecla_irq_event);
}


