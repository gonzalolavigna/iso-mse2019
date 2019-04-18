#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task2.h"
#include "task3.h"
#include "math.h"


//Estos datos se comparten externamente
uint32_t 						task3_stack[TASK3_STACK_SIZE_BYTES/4];
os_event_handler_t 	result_ready;

/*En este buffer se copian los datos provenientes del que escribe los datos*/
static int16_t 			buffer_data[BUFFER_SIZE];
static buffer_data_t buffer;

/*El resultado los pasamos a float_t para usar la parte de punbto flotante*/
float_t 			result_data[BUFFER_RESULT_SIZE];

void* task3 (void* a){
	float_t 	multiplier = 1.1;
	int16_t*  temp_pointer;
	uint32_t 	i;
	//Usamos el GPIO 2 para ver cuanto dura el copia del buffer
	//y la operacion en punto flotante sobre el
	gpioInit(GPIO2,GPIO_OUTPUT);
	buffer_init(&buffer,BUFFER_COPY,(void*)buffer_data,BUFFER_SIZE);
	while(1){
		gpioToggle(LEDB);
		//Esperamos al buffer A para que de resultados validos
		os_event_wait(buffer_a_ready);
		gpioToggle(GPIO2);
		//Copiamos el buffer sobre el cual vamos a trabajar
		buffer_copy(&buffer_a,&buffer);
		//Hacemos la operacion casteando a float el dato y operando
		for(i=0;i<buffer.buffer_size;i++){
			result_data[i] = multiplier * (float_t)(buffer.buffer_ptr[i]);
		}
		os_event_set(result_ready);
		gpioToggle(GPIO2);
		//Esperamos al buffer B para que de resultados validos
		os_event_wait(buffer_b_ready);
		gpioToggle(GPIO2);
		//Copiamos el buffer sobre el cual vamos a trabajar
		buffer_copy(&buffer_b,&buffer);
		//Hacemos la operacion casteando a float el dato y operando
		for(i=0;i<buffer.buffer_size;i++){
			result_data[i] = multiplier * (float_t)(buffer.buffer_ptr[i]);
		}
		os_event_set(result_ready);
		gpioToggle(GPIO2);
	}

}


void* hook3 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}
