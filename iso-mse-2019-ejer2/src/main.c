#include "sapi.h"
#include "sapi_circularBuffer.h"
#include "os.h"
#include <string.h>
#include "task1.h"
#include "task2.h"
#include "task3.h"



int main (void){
  /* Inicializar la UART_USB */
	uartInit(UART_USB,115200);
  //Mensaje de bienvenida antes de arrancar
	uartWriteString(UART_USB,"OS-ISO MSE 2019 Gonzalo Lavigna-Ejercicio 2\r\n");

	/*No sabemos como arrancan todas las colas con lo cual las inicializamos a todas con algo con algo conocido*/
	os_queue_init();
	/*Todos los eventos no savemos como arrancan y para no tener conflicto los inicializamos con algo conocido*/
	os_event_init_array();

	//Creo el evento de simular la IRQ de 6 ms
	if((irq_emulator_event = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:IRQ EMULATOR\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	//Creo el evento para señalizar que hay el buffer A READY
	if((buffer_a_ready = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:EVENT A READY EMULATOR\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	//Creo el evento para señalizar que hay el buffer B READY
	if((buffer_b_ready = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:EVENT B READY\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	//Creo el evento para señalizar que hay un resultado terminado
	if((result_ready = os_event_init()) == NULL){
		uartWriteString(UART_USB,"ERROR CREANDO EVENTO:EVENT B READY\r\n");
		while(1){
			//Si falla directamente esta creación me quedo esperando forever para poder verlo en un
			//debugger
			__WFI();
		}
	}

	/*Creación de las tarea que vamos a ejecutar, a la tarea task3 de la UART directamente
	 * le damos baja prioridad para que no absorva a la otra*/
	os_task_create(task1_stack,TASK1_STACK_SIZE_BYTES,task1,HIGH_PRIORITY	,(void*)0x11111111);
	os_task_create(task2_stack,TASK2_STACK_SIZE_BYTES,task2,HIGH_PRIORITY	,(void*)0x22222222);
	os_task_create(task3_stack,TASK3_STACK_SIZE_BYTES,task3,HIGH_PRIORITY	,(void*)0x33333333);

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
	while(1){
		/*Cada vez que entramos al hook que nos da el OS hacemos un gpioToggle del LED 3*/
		gpioToggle(LED3);
		__WFI();
	}
}



