#include <stdint.h>
#include "string.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"
#include "task1.h"
#include "task2.h"

//Declaracion de funciones internas
void buffer_write_constant (buffer_data_t* buffer,int16_t constant);
void buffer_write(buffer_data_t* buffer);

//Estos datos se comparten externamente
uint32_t 						task2_stack[TASK2_STACK_SIZE_BYTES/4];
os_event_handler_t 	buffer_a_ready;
os_event_handler_t 	buffer_b_ready;

/*Declaracion de datos estaticos*/
int16_t buffer_a_data[BUFFER_SIZE];
int16_t buffer_b_data[BUFFER_SIZE];
buffer_data_t buffer_a;
buffer_data_t buffer_b;

/*Contadores para guardar en constantes para buffer A par y para buffer Bimparr */
int16_t even_counter = 0;
int16_t odd_counter  = 1;


void* task2 (void* a){
	//Usamos el GPIO 1 para ver cuanto dura el llenado del buffer
	//Tambien para ver que tenemos la sincronizacion adecuada
	gpioInit(GPIO1,GPIO_OUTPUT);
	buffer_init(&buffer_a,BUFFER_A,(void*)buffer_a_data,BUFFER_SIZE);
	buffer_init(&buffer_b,BUFFER_B,(void*)buffer_b_data,BUFFER_SIZE);
	while(1){
		gpioToggle(LED2);
		os_event_wait(irq_emulator_event);
		gpioToggle(GPIO1);
		buffer_write(&buffer_a);
		os_event_set(buffer_a_ready);
		gpioToggle(GPIO1);
		os_event_wait(irq_emulator_event);
		gpioToggle(GPIO1);
		buffer_write(&buffer_b);
		os_event_set(buffer_b_ready);
		gpioToggle(GPIO1);
	}
}

void* hook2 (void* p) {
	while(1){
		__WFI();
	}
	return (void*)TRUE;
}

void buffer_write(buffer_data_t* buffer){
	if(buffer->buffer_id == BUFFER_A){
		buffer_write_constant(buffer,even_counter);
		even_counter+=2;
	}else if (buffer->buffer_id == BUFFER_B) {
		buffer_write_constant(buffer,odd_counter);
		odd_counter+=2;
	}else {
		//Basicamente matamos el O.S
		os_enter_critical();
		uartWriteString(UART_USB,"ERROR EN TAREA WRITE PING PONG BUFFER");
		__WFI();
	}
}


void buffer_init(buffer_data_t* buffer,buffer_id_t buffer_id, void* array, uint32_t size){
	buffer->buffer_id 	= buffer_id;
	buffer->buffer_lock = FALSE;
	//Casteo del arreglo para poder descansarlos
	buffer->buffer_ptr 	= (int16_t*)array;
	buffer->buffer_size =	size;
}

void buffer_write_constant (buffer_data_t* buffer,int16_t constant){
	uint32_t i;
	for(i=0;i<buffer->buffer_size;i++){
		buffer->buffer_ptr[i] = constant;
	}
	buffer->buffer_lock = TRUE;
}

void buffer_copy(buffer_data_t* src_buffer,buffer_data_t* dst_buffer){
	uint32_t i;
	//Copiamos el ID para llevar un track de los mismos
	dst_buffer->buffer_id = src_buffer->buffer_id;
	for(i=0;i<src_buffer->buffer_size;i++){
		//Hacemos una copia recorriendo los dos punteros
		dst_buffer->buffer_ptr[i] = src_buffer->buffer_ptr[i];
	}
	//Una vez que lo escribimos directamente pasa a FALSE
	src_buffer->buffer_lock = FALSE;
	//Una vez que lo terminamos de copiar el destino lo marcamos como lock
	dst_buffer->buffer_lock = TRUE;
}


