#ifndef __TASK_PRINT__
#define __TASK_PRINT__

#include <stdint.h>
#include "sapi.h"
#include "os.h"
#include "os_delay.h"
#include "os_event.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_PRINT_STACK_SIZE_BYTES 2048
extern 	uint32_t 	task_print_stack[];

extern os_event_handler_t event_led_green;
extern os_event_handler_t event_led_red;
extern os_event_handler_t event_led_yellow;
extern os_event_handler_t event_led_blue;

extern os_mutex_handler_t mutex_led;

void* task_print (void* a);
void* hook_print (void* p);

typedef enum {
	WAIT_FIRST_FALLING_EDGE = 0,
	WAIT_FALLING_EDGE_TEC_1,
	WAIT_FALLING_EDGE_TEC_2,
	WAIT_FIRST_RISING_EDGE,
	WAIT_RISING_EDGE_TEC_1,
	WAIT_RISING_EDGE_TEC_2,
}fsm_edge_measure_state_t;

typedef enum {
	GREEN_EVENT = 0,
	RED_EVENT ,
	YELLOW_EVENT,
	BLUE_EVENT,
	NOTHING,
}led_event_t;

typedef struct {
	uint32_t 									falling_edge_tick_tec_1;
	uint32_t 									falling_edge_tick_tec_2;
	uint32_t 									rising_edge_tick_tec_1;
	uint32_t 									rising_edge_tick_tec_2;
	uint32_t 									time_1;
	uint32_t 									time_2;
	fsm_edge_measure_state_t 	state;
	led_event_t 						 	led_event;
}edge_measure_data_t;

void init_edge_detector_fsm (void);
uint32_t get_led_delay_time (void);


#ifdef __cplusplus
}
#endif

#endif
