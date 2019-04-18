#ifndef __TASK2__
#define __TASK2__

#include <stdint.h>
#include "os.h"
#include "os_delay.h"
#include "os_event.h"
#include "sapi.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TASK2_STACK_SIZE_BYTES 	512
#define BUFFER_SIZE 						240

void* task2 (void* a);
void* hook2 (void* p);

//Identificamos asi a los distintos buffer
typedef enum {
	BUFFER_A = 0,
	BUFFER_B,
	BUFFER_COPY,
}buffer_id_t;

typedef struct{
	int16_t* 				buffer_ptr;
	uint32_t 				buffer_size;
	//Me sirve para detectar una condición de carrera
	bool_t 					buffer_lock;
	buffer_id_t		 	buffer_id;
}buffer_data_t;

//Exportamos los datos de los dos buffers para poder usarlos en otros bloques
extern buffer_data_t buffer_a,buffer_b;
//El stack se usa para inicializar la tarea
extern 	uint32_t 						task2_stack[];
//Mediante estos eventos indicamos que los buffer de escrituras
//estan listos
extern 	os_event_handler_t 	buffer_a_ready;
extern 	os_event_handler_t 	buffer_b_ready;


//Esta función esta pensada para copiar el buffer a otro y desbloquear
//el actual
void buffer_copy(buffer_data_t* src_buffer,buffer_data_t* dst_buffer);
void buffer_init(buffer_data_t* buffer,buffer_id_t buffer_id, void* array, uint32_t size);


#ifdef __cplusplus
}
#endif


#endif
