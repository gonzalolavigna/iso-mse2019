#include <stdio.h>
#include <string.h>
#include "math.h"
#include "sapi.h"
#include "sapi_circularBuffer.h"
#include "os.h"
#include "task_debounce.h"
#include "task_print.h"
#include "task_led_green.h"
#include "task_led_red.h"
#include "task_led_yellow.h"
#include "task_led_blue.h"
#include "utils.h"


int main (void){

	/* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
	uartInit(UART_USB,115200);

	/*No sabemos como arrancan todas las colas con lo cual las inicializamos a todas con algo con algo conocido*/
	os_queue_init();
	/*Todos los eventos no savemos como arrancan y para no tener conflicto los inicializamos con algo conocido*/
	os_event_init_array();
	/*Inicializamos todos los mutex para que arranquen todos en el mismo valor*/
	os_mutex_init_array();

	//Creo el evento de la tecla antes de lanzar el OS --> si es NULL me quedo esperando
	if((tecla_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:TECLA EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	//Inicializamos los eventos para prender los leds los hacemos en distintas tareas para poder tener
	//mejor tiempo de reaccion.
	if((event_led_green = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED VERDE EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((event_led_red = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED ROJO EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((event_led_yellow = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED AMARILLO EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((event_led_blue = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED AZUL\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((mutex_led = os_mutex_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO MUTEX PARA PROTEGER DATOS HACIA LOS LEDS\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	if((mutex_tecla = os_mutex_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO MUTEX PARA PROTEGER DATOS DE LAS TECLAS\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	/*Creación de las tarea que vamos a ejecutar, a la tarea task3 de la UART directamente
	 * le damos baja prioridad para que no absorva a la otra*/
	os_task_create(task_debounce_stack  	,TASK_DEBOUNCE_STACK_SIZE_BYTES 	,task_debounce   ,HIGH_PRIORITY		,(void*)0x11111111);
	os_task_create(task_print_stack     	,TASK_PRINT_STACK_SIZE_BYTES    	,task_print      ,MEDIUM_PRIORITY	,(void*)0x22222222);
	os_task_create(task_led_green_stack 	,TASK_LED_GREEN_STACK_SIZE_BYTES	,task_green      ,HIGH_PRIORITY  	,(void*)0x33333333);
	os_task_create(task_led_red_stack   	,TASK_LED_RED_STACK_SIZE_BYTES  	,task_red        ,HIGH_PRIORITY  	,(void*)0x44444444);
	os_task_create(task_led_yellow_stack  ,TASK_LED_YELLOW_STACK_SIZE_BYTES ,task_yellow     ,HIGH_PRIORITY  	,(void*)0x55555555);
	os_task_create(task_led_blue_stack    ,TASK_LED_BLUE_STACK_SIZE_BYTES   ,task_blue       ,HIGH_PRIORITY  	,(void*)0x66666666);

	/*Inicializamos el O.S*/
	os_init();

	/*Despues del primer cambio de contexto no volvemos mas aca y loopeamos entre las tareas*/
	while(1){
		__WFI();
	}

	return TRUE;
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
		//gpioToggle(LED3);
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




