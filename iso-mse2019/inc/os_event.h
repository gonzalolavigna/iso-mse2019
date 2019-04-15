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

typedef void * os_event_handler_t;

typedef enum {
	EVENT_INIT = 0,
	EVENT_WAIT,
	EVENT_SET ,
} event_state_t;


typedef struct {
	event_state_t 	state;
	uint32_t 				task_waiting;
} event_t;


void 								os_event_init_array(void);
os_event_handler_t 	os_event_init	(void);
bool_t 							os_event_wait	(os_event_handler_t event);
bool_t 							os_event_set	(os_event_handler_t event);



/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
