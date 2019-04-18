#ifndef __OS_DELAY__
#define __OS_DELAY__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "task_stack.h"


/*==================[inclusiones]============================================*/
/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif


/*==================[declaraciones de funciones externas]====================*/
bool_t 		os_task_delay					(uint32_t ticks);
uint32_t 	os_get_ticks_running	(void);
void 			enable_sys_tick_irq		(void);
void 			disable_sys_tick_irq	(void);


/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif
/*==================[end of file]============================================*/
#endif
