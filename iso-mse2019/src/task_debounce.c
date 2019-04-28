#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "os_queue.h"
#include "sapi.h"
#include "task_debounce.h"


//Stack de la tarea
uint32_t task_debounce_stack[TASK_DEBOUNCE_STACK_SIZE_BYTES/4];
//Con este evento avisamos que tenemos algun evento de alguna tecla
os_event_handler_t 	tecla_event;
os_mutex_handler_t  mutex_tecla;
os_queue_handler_t  cola_irq_teclas;

//Inicializacion de las IRQ de GPIO
void init_irq_gpio(void);
//Esta funcion es igual para todas las IRQ y simplifica el envio de eventos al OS.
void serve_gpio_irq(uint32_t tecla_index,edge_t edge);
//Funciones para ser utilizadas por otras tareas
void 	debounce_init				(void);



//Uso las teclas 2 y las teclas 4 ya que las adyacende como TEC 1 y TEC 2 no puedo por los tamaño de mis dedos
gpioMap_t teclas[] = {TEC2,TEC4};
#define TECLAS_VALIDAS sizeof(teclas)/sizeof(gpioMap_t)
debounce_data_t tecla_array[TECLAS_VALIDAS];

//Creamos el arreglo para poner la informacion de la tecla
//La idea de aqui es ir encolando interrupciones de las teclas para realizar el debounce
//El debounce requiere unos os_task_delay del orden de 10 ms por lo tanto con una cola de
//mensajes no nos vamos a peder datos de las IRQ
tecla_irq_data_t  tecla_irq_data[OS_QUEUE_TECLA_IRQ_SIZE];

bool_t cola_teclas_init(void){
	//Inicio la cola del OS con los datos del buffer circular creado para esta cola de mensajes.
	cola_irq_teclas = os_message_queue_init((void*)&tecla_irq_data[0],OS_QUEUE_TECLA_IRQ_SIZE,sizeof(tecla_irq_data_t));
	if(cola_irq_teclas == NULL){
		return FALSE;
	}
	return TRUE;
}

//Inicializacion de los datos para hacer el debounce de las teclas
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
	bool_t 	 hit = FALSE;
	uint32_t i;
	static tecla_irq_data_t irq_data_temp;
	//Inicializamos los datos para la inicialización del debouncer.
	debounce_init();
	//Inicializamos la interrupciones para las teclas una vez inicializada la estructura de datos.
	init_irq_gpio();
	while(1){
		//Nos quedamos bloqueados hasta que las IRQ de las colas coloquen algo en la cola del OS.
		os_queue_get(cola_irq_teclas,(void*)&irq_data_temp);
		//Por defecto no hay informacion que pasar a la tarea que realiza el print.
		hit = FALSE;
		//Estos datos los puedo sacar de tarea de print por lo tanto los protegemos
		os_mutex_lock(mutex_tecla);
		//Reseteo esta informacion de eventos de flanco para pasarla a la FSM que decide la tecla pulsada.
		for(i=0;i<TECLAS_VALIDAS;i++){
			tecla_array[i].tecla_falling_event = FALSE;
			tecla_array[i].tecla_rising_event  = FALSE;
		}
		//Recorremos el estado de la tecla que tuvo una interrupcion.
		switch(tecla_array[irq_data_temp.tecla_index].state){
		case BUTTON_UP:
			//Si la tecla esta en soltada y se detecta un flanco ascendente no es correcto, ignoramos este suceso.
			if(irq_data_temp.edge == FALLING_EDGE){
				tecla_array[irq_data_temp.tecla_index].tecla_falling_event = TRUE;
				tecla_array[irq_data_temp.tecla_index].tecla_falling_tick  = irq_data_temp.tick;

				//Hago el correspondiente debounce con un delay, lo bueno de tener un cola que no pierdo eventos.
				os_task_delay(DEBOUNCE_TICKS);
				if(!gpioRead(tecla_array[irq_data_temp.tecla_index].tecla)){
					//La FSM pasa a BUTTON DOWN
					tecla_array[irq_data_temp.tecla_index].state = BUTTON_DOWN;
					hit = TRUE;
				}else {
					tecla_array[irq_data_temp.tecla_index].state = BUTTON_UP;
					hit = FALSE;
				}
			}
			break;
			//Si la tecla esta presionada y se detecta un flanco descendente no es correcto, ignoramos este suceso.
		case BUTTON_DOWN:
			if(irq_data_temp.edge == RISING_EDGE){
				tecla_array[irq_data_temp.tecla_index].tecla_rising_event = TRUE;
				tecla_array[irq_data_temp.tecla_index].tecla_rising_tick  =  irq_data_temp.tick;
				//Hago el correspondiente debounce con un delay, lo bueno de tener un cola que no pierdo eventos.
				os_task_delay(DEBOUNCE_TICKS);
				if(gpioRead(tecla_array[irq_data_temp.tecla_index].tecla)){
					//La FSM pasa a BUTTON DOWN
					tecla_array[irq_data_temp.tecla_index].state = BUTTON_UP;
					hit = TRUE;
				}else {
					tecla_array[irq_data_temp.tecla_index].state = BUTTON_DOWN;
					hit = FALSE;
				}
			}
			break;
		default:
			//TODO:Analizar en un futuro agregar una condición de salida por este error.
			break;
		}
		//Ya no manejo mas los datos criticos de la tecla con lo cual ya dejamos de protegerlos, para evitar colisiones.
		os_mutex_unlock(mutex_tecla);
		//Si hay algo que informar se manda un evento a la tarea de task print con es evento.
		if(hit == TRUE){
			os_event_set(tecla_event);
		}
	}
}

//Esta funcion va a ser utilizada por la tarea task print para obtener de una manera rapida los datos
///TODO: Reemplazar eventualmente con cola de mensajes del OS.
//Generalmente esta tarea va a correr en el entorno de otra tarea que necesie los datos de todas las teclas.
void copy_tecla_array(debounce_data_t * dst ){
	uint32_t i;
	//Estos datos los puedo sacar de otra tarea por lo tanto nos protegemos
	os_mutex_lock(mutex_tecla);
	//Esto hace una copia rapida es mucho mejor que usar en la task print
	for(i=0;i<TECLAS_VALIDAS;i++){
		dst[i].state 			 						= tecla_array[i].state;
		dst[i].tecla 			 						= tecla_array[i].tecla;
		dst[i].tecla_falling_event 		= tecla_array[i].tecla_falling_event;
		dst[i].tecla_rising_event 		= tecla_array[i].tecla_rising_event;
		dst[i].tecla_falling_tick 		= tecla_array[i].tecla_falling_tick;
		dst[i].tecla_rising_tick  		= tecla_array[i].tecla_rising_tick;
	}
	//YA termine de usar los datos criticos, con lo cual desactivo el mutex.
	os_mutex_unlock(mutex_tecla);
}

//TODO: Aun no implementado en nuestro OS.
void* task_debounce_hook (void* p){
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

//Inicializamos las interrupciones de las teclas.
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

	//Configuro prioridad de interrupciones le pongo la prioridad 1 mas que la pendsv
	//COn lo cual no se va a ejecutar nunca un contexto mientras se atiende las IRQ de los flancos.
	NVIC_SetPriority(PIN_INT0_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT1_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT2_IRQn,(1 << __NVIC_PRIO_BITS) - 2);
	NVIC_SetPriority(PIN_INT3_IRQn,(1 << __NVIC_PRIO_BITS) - 2);

	//Limpiamos las interrupciones
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

//Esta funcion escribe en la cola de mensajes del OS para destrabar la tarea debounce para guardar los datos de las teclas.
void serve_gpio_irq(uint32_t tecla_index,edge_t edge){
	//Dato static para que esto funcione mas rapido, puede que no sea necesario.
	static tecla_irq_data_t irq_data;
	//La tarea que hace el debouncing necesita esta informacion para hacer avanzar su informacion.
	irq_data.tecla_index  = tecla_index;
	irq_data.edge 				= edge;
	irq_data.tick    			= os_get_ticks_running();
	//Esto funciona con copia la cola del O.S, tiene proteccion con mutex por concurrencia.
	//Como es una ISR obviamente no es bloqueante esta funcion.
	//El rv de esta funcion actualmente se esta ignorando.
	os_queue_put_from_isr(cola_irq_teclas,(void*)&irq_data);
}


