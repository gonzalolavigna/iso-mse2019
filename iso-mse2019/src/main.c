#include <stdio.h>
#include <string.h>
#include "math.h"
#include "sapi.h"
#include "sapi_circularBuffer.h"
#include "os.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"
#include "task5.h"
#include "task6.h"
#include "utils.h"


void onRx (void *noUsado);

int main (void){

	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	//Solo vamos a usar la de RX para no perdernos eventos
	uartInit(UART_USB,115200);
  // Seteo un callback al evento de recepcion y habilito su interrupcion
  uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
  // Habilito todas las interrupciones de UART_USB
  uartInterrupt(UART_USB, true);


  //Mensaje de bienvenida antes de arrancar
	uartWriteString(UART_USB,"OS-ISO MSE 2019 Gonzalo Lavigna\r\n");


  //Inicializamos el buffer circular de la uart
  init_uart_circular_buffer();
	/*No sabemos como arrancan todas las colas con lo cual las inicializamos a todas con algo con algo conocido*/
	os_queue_init();
	/*Todos los eventos no savemos como arrancan y para no tener conflicto los inicializamos con algo conocido*/
	os_event_init_array();
	/*Inicializamos todos los mutex para que arranquen todos en el mismo valor*/
	os_mutex_init_array();

	//Creo el evento de la tecla antes de lanzar el OS --> si es NULL me quedo esperando
	if((tecla_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:TECLA\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((tecla_1_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:TECLA 1\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((uart_mutex = os_mutex_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO MUTEX:UART\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}


	if((uart_receive_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO: RECEPION UART\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}


	/*Creación de las tarea que vamos a ejecutar, a la tarea task3 de la UART directamente
	 * le damos baja prioridad para que no absorva a la otra*/
	os_task_create(task1_stack,TASK1_STACK_SIZE_BYTES,task1,HIGH_PRIORITY		,(void*)0x11111111);
	os_task_create(task2_stack,TASK2_STACK_SIZE_BYTES,task2,HIGH_PRIORITY		,(void*)0x22222222);
	os_task_create(task3_stack,TASK3_STACK_SIZE_BYTES,task3,LOW_PRIORITY		,(void*)0x33333333);
	os_task_create(task4_stack,TASK4_STACK_SIZE_BYTES,task4,HIGH_PRIORITY		,(void*)0x44444444);
	os_task_create(task5_stack,TASK5_STACK_SIZE_BYTES,task5,LOW_PRIORITY  	,(void*)0x55555555);
	os_task_create(task6_stack,TASK6_STACK_SIZE_BYTES,task6,MEDIUM_PRIORITY	,(void*)0x66666666);

	/*Inicializamos el O.S*/
	os_init();

	/*Despues del primer cambio de contexto no volvemos mas aca y loopeamos entre las tareas*/
	while(1){
		__WFI();
	}

	return TRUE;
}

/*Tarea para atajar la interrupcion de recepcion de una uart y ponerlo en un buffer tipo SAPI**/
void onRx (void *noUsado){
	static char c;
	c= uartRxRead(UART_USB);
	//Mensaje de Debug
	//	printf( "Recibimos <<%c>> por UART\r\n", c );
	circularBufferWrite(&uart_buffer_sapi,&c);
	os_event_set_from_irq(uart_receive_event);
}

/*Prendemos algun LED para indicar que estamos haciendo cambio a IDLE
 * Creo que menos luminosidad nos indicara que tanto tenemos ocupado el O.S*/
void* 	idle_task(void * arg){
	uint32_t i;
	static uint32_t task_context_switch_count[MAX_TASK_COUNT];
	static uint32_t os_context_switch_count;
	static uint32_t idle_context_switch_count;
	static uint32_t task_count;
	static float 	 f;
	static uint8_t   print_float_number[16];

	task_count = os_get_task_count();
	printf("Tareas Creadas:%d\r\n",task_count);
	while(1){

		/*Cada vez que entramos al hook que nos da el OS hacemos un gpioToggle del LED 3*/
		gpioToggle(LED3);
		/*Obtengo los datos para printearlos con la tarea idle*/
		idle_context_switch_count = os_get_idle_contex_given_counter();
		os_context_switch_count = os_get_os_context_switch_counter();
		for(i=0;i<task_count;i++){
			task_context_switch_count[i] = os_get_task_context_given_counter(i);
		}
		printf("CONTEXT SWITCH OS:%d\r\n",os_context_switch_count);
		for(i=0;i<task_count;i++){
			convert_float_to_str(print_float_number,100*(float)task_context_switch_count[i]/os_context_switch_count,2);
			printf("TASK%d CONTEXT GIVEN:%d PORCENTAJE:%s\r\n",i+1,task_context_switch_count[i],print_float_number);
  		}
		convert_float_to_str(print_float_number,100*(float)idle_context_switch_count/os_context_switch_count,2);
		printf("IDLE CONTEXT GIVEN:%d PORCENTAJE:%s \r\n",idle_context_switch_count,print_float_number);
		//Esperamos 1 segundo con una funcion bloqueante que no hace uso del Sistick
		delayInaccurateMs(10000);
		__WFI();
	}
}




