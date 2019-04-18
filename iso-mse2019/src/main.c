#include "sapi.h"
#include "os.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "task4.h"
#include "task5.h"
#include <string.h>


int main (void){
	uartInit(UART_USB,115200);
	uartWriteString(UART_USB,"OS-ISO MSE 2019 Gonzalo Lavigna\r\n");

	/*No sabemos como arrancan todas las colas con lo cual las inicializamos a todas con algo con algo conocido*/
	os_queue_init();
	/*Todos los eventos no savemos como arrancan y para no tener conflicto los inicializamos con algo conocido*/
	os_event_init_array();

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
	/*Creación de las tarea que vamos a ejecutar, a la tarea task3 de la UART directamente
	 * le damos baja prioridad para que no absorva a la otra*/
	os_task_create(task1_stack,TASK1_STACK_SIZE_BYTES,task1,HIGH_PRIORITY		,(void*)0x11111111);
	os_task_create(task2_stack,TASK2_STACK_SIZE_BYTES,task2,HIGH_PRIORITY		,(void*)0x22222222);
	os_task_create(task4_stack,TASK4_STACK_SIZE_BYTES,task4,HIGH_PRIORITY		,(void*)0x44444444);
	os_task_create(task5_stack,TASK5_STACK_SIZE_BYTES,task5,MEDIUM_PRIORITY	,(void*)0x55555555);
	os_task_create(task3_stack,TASK3_STACK_SIZE_BYTES,task3,LOW_PRIORITY		,(void*)0x33333333);

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
	gpioInit(GPIO5,GPIO_OUTPUT);
	gpioWrite(GPIO5,ON);
	while(1){
		/*Cada vez que entramos al hook que nos da el OS hacemos un gpioToggle del LED 3*/
		gpioToggle(LED3);
		gpioToggle(GPIO5);
		__WFI();
	}
}
