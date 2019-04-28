#ifndef __OS_QUEUE__
#define __OS_QUEUE__

#include <stdint.h>
#include "sapi.h"

#include "os.h"
#include "os_event.h"

/*==================[inclusiones]============================================*/
/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	//Para proteger cuando quedo en empty y en full
	os_event_handler_t event;
	//Protejo la concurrencia de eventos
	os_mutex_handler_t mutex;
	//Esta es la estructura de conrtrol para el buffer circular de la SAPI
	circularBuffer_t 	 buffer;
} os_queue_t;

typedef os_queue_t* os_queue_handler_t;

void os_queue_init_array(void);

os_queue_handler_t os_message_queue_init( void* memory,uint32_t elements,uint32_t element_size);
//Para escribir le pasamos un puntero hacia donde esta la informacion.
bool_t os_queue_get(os_queue_handler_t queue, void* data);
//Para ponerlo le pasamos un puntero a los datos
bool_t os_queue_put(os_queue_handler_t queue, void* data);
//Para poner datos desde una interrupcion
bool_t os_queue_put_from_isr(os_queue_handler_t queue, void* data);
//Funcion para obtener cuantas colas fueron creadas por el usuario.
uint32_t os_get_queue_count(void);



/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
