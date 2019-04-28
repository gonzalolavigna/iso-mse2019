#include <stdio.h>
#include <string.h>
#include "math.h"
#include "sapi.h"
#include "sapi_circularBuffer.h"
#include "os.h"
#include "os_queue.h"
#include "os_event.h"
#include "task_debounce.h"
#include "task_print.h"
#include "task_led_green.h"
#include "task_led_red.h"
#include "task_led_yellow.h"
#include "task_led_blue.h"
#include "utils.h"

//#define DEBUG_MODE

int main (void){

	//Inicializamos la UART para imprimir en las distintas tareas del O.S
	uartInit(UART_USB,115200);
	uartWriteString(UART_USB,"ISO-MSE-2019:Final-Gonzalo Lavigna 28/04/2018 \r\n");
	printf("Maximo Numero de Tareas permitidas por nuestro OS:%d\r\n",MAX_TASK_COUNT);
	printf("Maximo Numero de Prioridades permitidas por nuestro OS: 3 HIGH-MEDIUM-LOW\r\n");
	printf("Maximo Numero de Eventos permitidos por nuestro OS:%d\r\n",MAX_EVENT_COUNT);
	printf("Maximo Numero de Mutex permitidos por nuestro OS:%d\r\n",MAX_MUTEX_COUNT);
	printf("Maximo Numero de Colas OS permitidas por nuestro OS:%d\r\n",MAX_QUEUE_COUNT);
	printf("TECLA 2 EDU CIAA EQUIVALE a B1 del examen\r\n");
	printf("TECLA 4 EDU CIAA EQUIVALE a B2 del examen\r\n");
	printf("Descomentar linea 19 del archivo main.c --> #define DEBUG_MODE para permitir la impresion de utilizacion de tareas cada 15 segundo aproximadamente\r\n");
	printf("\r\n\r\n\r\n");



	///TODO: Esta inicializacion es obligatoria, pero se debe ocular al usuario.
	/*No sabemos como arrancan todas las colas de prioridades de tarea con lo cual las inicializamos a todas con algo con un valor inicial*/
	os_priority_queue_init();
	/*Todos los eventos no savemos como arrancan y para no tener conflicto los inicializamos con algo conocido*/
	os_event_init_array();
	/*Inicializamos todos los mutex para que arranquen todos en el mismo valor*/
	os_mutex_init_array();
	/*Inicializamos todas las colas de mensajes con vectores NULL y punteros a mutex y eventos nulos*/
	os_queue_init_array();

	//Creo el evento de la tecla antes de lanzar el OS --> si es NULL me quedo esperando
	if((tecla_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:TECLA EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debuggger
			__WFI();
		}
	}
	//Para que pueda prender un led sin correlacion con el otro y por ejemplo permitir el encendido de un led
	//con un delay de 1 segundo y otro de 100 ms. Estan desacoplados los eventos.
	if((event_led_green = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED VERDE EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}
	if((event_led_red = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED ROJO EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}
	if((event_led_yellow = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED AMARILLO EVENT\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}

	if((event_led_blue = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:LED AZUL\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}
	//Este mutex proteje el flujo de informacion entre la tarea que tiene la FSM de las teclas
	//y las distintas tareas de los leds.
	if((mutex_led = os_mutex_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO MUTEX PARA PROTEGER DATOS HACIA LOS LEDS\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}
	//Este mutex proteje los datos entre la tarea que realiza el debounce de las teclas y la FSM
	//de las teclas.
	if((mutex_tecla = os_mutex_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO MUTEX PARA PROTEGER DATOS DE LAS TECLAS\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en el debugger
			__WFI();
		}
	}
	//Esta funcion esta embebida en task_debounce.c para simplificar la inicialización de una cola del OS
	if(cola_teclas_init() == FALSE){
		uartWriteString(UART_USB,"ERROR CREANDO COLA DE MENSAJES DEL OS\r\n");
	}

	//Son todas de alta prioridad menos la que realiza el print ya que puede durar muchos ticks, y si sucede algo quiero que
	//la tarea del debounce o los leds lo hagan.
	os_task_create(task_debounce_stack  	,TASK_DEBOUNCE_STACK_SIZE_BYTES 	,task_debounce   ,HIGH_PRIORITY		,(void*)0x11111111);
	os_task_create(task_print_stack     	,TASK_PRINT_STACK_SIZE_BYTES    	,task_print      ,MEDIUM_PRIORITY	,(void*)0x22222222);
	os_task_create(task_led_green_stack 	,TASK_LED_GREEN_STACK_SIZE_BYTES	,task_green      ,HIGH_PRIORITY  	,(void*)0x33333333);
	os_task_create(task_led_red_stack   	,TASK_LED_RED_STACK_SIZE_BYTES  	,task_red        ,HIGH_PRIORITY  	,(void*)0x44444444);
	os_task_create(task_led_yellow_stack  ,TASK_LED_YELLOW_STACK_SIZE_BYTES ,task_yellow     ,HIGH_PRIORITY  	,(void*)0x55555555);
	os_task_create(task_led_blue_stack    ,TASK_LED_BLUE_STACK_SIZE_BYTES   ,task_blue       ,HIGH_PRIORITY  	,(void*)0x66666666);

	/*Inicializamos el O.S*/
	os_init();

	/*Despues del primer cambio de contexto no volvemos mas aca y hacemos distintas cosas con las correspondientes tareas*/
	while(1){
		__WFI();
	}

	return TRUE;
}



//Esta tarea
void* 	idle_task(void * arg){
	uint32_t i;
	static uint32_t task_context_switch_count[MAX_TASK_COUNT];
	static uint32_t os_context_switch_count;
	static uint32_t idle_context_switch_count;
	static uint32_t task_count;
	static float 	 f;
	static uint8_t   print_float_number[16];

	task_count = os_get_task_count();
	printf("IDLE TASK:Tareas Creadas:%d\r\n",task_count);
	printf("IDLE TASK:Eventos Creadas por el usuario:%d\r\n",os_get_event_count());
	printf("IDLE TASK:Mutexes Creados por el usuario:%d\r\n",os_get_mutex_count());
	printf("IDLE TASK:Colas Creados por el usuario:%d\r\n",os_get_queue_count());
	printf("\r\n\r\n\r\n");
	while(1){
#ifdef DEBUG_MODE
		//Obtengo los datos para printearlos con la tarea idle
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
#endif
		__WFI();
	}
}

