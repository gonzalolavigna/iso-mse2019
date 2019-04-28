#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "os_queue.h"
#include "sapi.h"
#include "sapi_circularBuffer.h"

os_queue_t 	queue_list[MAX_QUEUE_COUNT];
uint32_t 		queue_count = 0;


void os_queue_init_array(void){
	uint32_t i;
	for(i=0;i<MAX_QUEUE_COUNT;i++){
		queue_list[i].event = NULL;
		queue_list[i].mutex = NULL;
	}
}


os_queue_handler_t os_message_queue_init( void* memory,uint32_t elements,uint32_t element_size){
	os_queue_handler_t queue_handler;
	//Si queremos crear un evento mas de lo que podemos directamente devolvemos NULL
	if(queue_count >= MAX_EVENT_COUNT ){
		return NULL;
	}
	//Inicializo el semaforo de la cola este es para despertar y apagar tareas cuando esta en full o en empty
	if((queue_list[queue_count].event = os_event_init()) == NULL){
		return NULL;
	}
	//Inicializo el mutex de la cola va a permitir que no se hagan accesos concurrente al buffer de la SAPI
	if((queue_list[queue_count].mutex = os_mutex_init()) == NULL){
		return NULL;
	}
	//Inicializo el buffer circular de la SAPi
	circularBuffer_Init(&(queue_list[queue_count].buffer),(uint8_t*)memory,elements,element_size);

	//Devuelvo el puntero a la cola para que pueda utilizarse por cualquier tarea
	queue_handler = &queue_list[queue_count];
	//Este contador para saber que no me pase de eventos estaticos.
	queue_count++;
	return queue_handler;
}

bool_t os_queue_get(os_queue_handler_t queue, void* data){
	//Casteo a un puntero a cola
	os_queue_t* local_queue = (os_queue_t*) queue;
	circularBufferStatus_t circular_buffer_status;
	do {
		os_mutex_lock(local_queue->mutex);
		//Me llevo los datos de la cola
		circular_buffer_status=circularBufferRead(&local_queue->buffer,(uint8_t*)data);
		os_mutex_unlock(local_queue->mutex);

		if(circular_buffer_status == CIRCULAR_BUFFER_EMPTY){
			//Pongo la tarea a dormir hasta que haya información
			os_event_wait(local_queue->event);
		} else if (circular_buffer_status == CIRCULAR_BUFFER_NORMAL){
			//Si es normal mando el evento si alguna tarea esta dormida la misma se despierta.
			os_event_set(local_queue->event);
		}else {
			uartWriteString(UART_USB,"OS QUEUE: Problama en GET");
			os_error_hook();
		}
	} while(circular_buffer_status == CIRCULAR_BUFFER_EMPTY);

	return TRUE;
}


bool_t os_queue_put(os_queue_handler_t queue, void* data){
	os_queue_t* local_queue = (os_queue_t*) queue;
	circularBufferStatus_t circular_buffer_status;

	do {
		os_mutex_lock(local_queue->mutex);
		//Me llevo los datos de la cola
		circular_buffer_status=circularBufferWrite(&local_queue->buffer,(uint8_t*)data);
		os_mutex_unlock(local_queue->mutex);

		if(circular_buffer_status == CIRCULAR_BUFFER_FULL){
			os_event_wait(local_queue->event);
		}else if(circular_buffer_status == CIRCULAR_BUFFER_NORMAL){
			os_event_set(local_queue->event);
		}else{
			uartWriteString(UART_USB,"OS QUEUE: Problama en PUT");
			os_error_hook();
		}
	} while(circular_buffer_status == CIRCULAR_BUFFER_FULL);
	return TRUE;
}

//Esta tarea es similar pero si el buffer esta full va a retornar FALSE, y si estan en funcionamiento normal
//va a enviar un evento para despertar las tareas que estan dormidas.
bool_t os_queue_put_from_isr(os_queue_handler_t queue, void* data){
	os_queue_t* local_queue = (os_queue_t*) queue;
	circularBufferStatus_t circular_buffer_status;

	//Que problema tenemos si estaba tomado el mutex asi que devolvemos un FALSE.
	if(os_mutex_lock_from_isr(local_queue->mutex) == FALSE){
		return FALSE;
	}
	//Escribimos los datos de la cola
	circular_buffer_status=circularBufferWrite(&local_queue->buffer,(uint8_t*)data);
	os_mutex_unlock_from_isr(local_queue->mutex);

	//Este es el valor de retorno la isr vera que hacer
	if(circular_buffer_status == CIRCULAR_BUFFER_FULL || circular_buffer_status == CIRCULAR_BUFFER_EMPTY ){
		return FALSE;
	}else{
		os_event_set_from_irq(local_queue->event);
		return TRUE;
	}
}

///TODO: No se si tiene sentido un metodo que sea os_queue_receive from ISR
//Una ISR siempre swe va a llevar lo que haya


uint32_t os_get_queue_count(void){
	return queue_count;
}







