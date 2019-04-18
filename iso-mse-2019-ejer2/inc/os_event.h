#ifndef __OS_EVENT__
#define __OS_EVENT__

#include <stdint.h>
#include "sapi.h"
#include "os.h"

/*==================[inclusiones]============================================*/
/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*De FreeRtos cuando creamos un evento devolvemos un puntero a ese
 * evento*/
typedef void * os_event_handler_t;

/*Un evento esta en wait cuando una tarea hace un os_event_wait()
 * Cuando le hacen os_event_Set() pasa a set*/
typedef enum {
	EVENT_INIT = 0,
	EVENT_WAIT,
	EVENT_SET ,
} event_state_t;


/*Un evento tiene un estado y a la tarea que es trabando*/
typedef struct {
	event_state_t 	state;
	uint32_t 				task_waiting;
} event_t;

/*Inicializa todos los eventos con valores conocidos*/
void 								os_event_init_array(void);
/*Inicializar un evento nos devuelve un handler al mismo, si devuelve un NULL no se pudo crear */
os_event_handler_t 	os_event_init	(void);
/*La tarea que lo llama queda bloqueada hasta que le venga un set del evento*/
bool_t 							os_event_wait	(os_event_handler_t event);
/*Esto tiene que ser llamado por otra tarea para darle un give y destrabar a la que llamo al wait*/
bool_t 							os_event_set	(os_event_handler_t event);



/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
