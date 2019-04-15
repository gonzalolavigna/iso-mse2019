#include "os.h"
#include "os_delay.h"
#include <stdint.h>
#include "string.h"
#include "sapi.h"
#include "task_stack.h"

//Numero maximo de las tareas permitidas por nuestro OS
#define MAX_TASK_COUNT 10
//Numero maximo de las colas de prioridades que admite nuestro OS
//Obivamente tengo que sacar la definicion del idle.
#define MAX_PRIORITY_QUEUE (sizeof(task_priority_t)-1)
//Ponemos un tamaño minimo del stack sin este no podemos crear un stack
#define STACK_MIN_SIZE 20
//Este es el stack asignado al contexto idle
#define IDLE_TASK_SIZE_BYTES 512

/*Variable globales del OS*/

task_context_t task_list[MAX_TASK_COUNT];
task_context_t idle_contex;

task_stack_t priority_queue[MAX_PRIORITY_QUEUE];

uint32_t task_count = 0;
uint32_t running_task_index = 0;
uint32_t idle_task_stack[IDLE_TASK_SIZE_BYTES / 4];
os_state_t os_state = OS_INIT;

void init_task_stack(uint32_t stack[], uint32_t stack_size_bytes,
    uint32_t *stack_pointer, task_type_f entry_point, void * arg);
void* idle_task(void * arg);
void os_error_hook(void);
void task_return_hook(void * ret_val);
bool_t search_next_task(uint32_t* task_index);

/*Declaracion de funciones externas utilizadas por el SO*/

bool_t os_queue_init(void) {
	uint32_t i;
	i = MAX_PRIORITY_QUEUE;
	for (i = 0; i < MAX_PRIORITY_QUEUE; i++) {
		task_stack_init(&priority_queue[i]);
	}
	return TRUE;
}

bool_t os_task_create(uint32_t stack[], uint32_t stack_size_bytes,
    task_type_f entry_point, task_priority_t priority, void * arg) {

	//Reviso que no tenga mas tareas de las que puedo manejar.
	//Reviso que el stack que me estan pasando no sea menor a 20 porque sino no
	//puedo crear un stack para el tamaño de los registros
	//Reviso que el stack sea divisible por 4, ya que las cuentas se hacen con ese valor
	if (task_count >= MAX_TASK_COUNT || stack_size_bytes < 20
	    || stack_size_bytes % 4)
		return FALSE;

	task_list[task_count].state = TASK_READY;
	task_list[task_count].priority = priority;
	task_list[task_count].reamaining_ticks = 0;
	task_list[task_count].initial_stack_pointer = stack;
	task_list[task_count].stack_size_bytes = stack_size_bytes;
	task_list[task_count].task_index = task_count;

	//Inicializo el stack y ya queda actualizado el stack pointer al lugar donde tengo el stack
	//para ejecutar la tarea
	init_task_stack(stack, stack_size_bytes, &task_list[task_count].stack_pointer,
	    entry_point, arg);

	//Pongo en la cola correspondiente el indice a la tarea
	task_stack_push(&priority_queue[priority], task_count);
	//Incremento la cantidad de tarea en el scheduler
	task_count++;

	return TRUE;
}


bool_t os_init(void) {
	boardConfig();
	SystemCoreClockUpdate();
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	SysTick_Config(SystemCoreClock / 1000);

	//Configuracion de la tarea IDLE
	idle_contex.state = TASK_READY;
	idle_contex.priority = IDLE_PRIORITY;
	idle_contex.reamaining_ticks = 0;
	idle_contex.initial_stack_pointer = idle_task_stack;
	idle_contex.stack_size_bytes = IDLE_TASK_SIZE_BYTES;
	idle_contex.task_index = 0xFFFFFFFF;
	init_task_stack(idle_task_stack,
	IDLE_TASK_SIZE_BYTES, &idle_contex.stack_pointer, idle_task,
	    (void *) 0x99999999);

	/*GPIO 3 -> GET NEXT CONTEXT GPIO 4 -> SYSTICK*/
	gpioInit(GPIO3, GPIO_OUTPUT);
	gpioInit(GPIO4, GPIO_OUTPUT);

	return TRUE;
}

void* idle_task(void * arg) {
	while (1) {
		__WFI();
	}
	return 0;
}
void task_return_hook(void * ret_val) {
	while (1) {
		__WFI();
	}
}

void os_error_hook(void) {
	while (1) {
		__WFI();
	}
}

uint32_t get_next_context(uint32_t current_sp) {
	uint8_t 	i;
	bool_t 		task_hit = FALSE;
	uint32_t 	next_stack_pointer;
	uint32_t 	task_index;

	gpioToggle(GPIO3);
	disable_sys_tick_irq();
	switch (os_state) {
	case OS_INIT:
		task_hit = search_next_task(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_RUNNING_TASK;
			task_list[task_index].state = TASK_RUNNING;
			next_stack_pointer = task_list[task_index].stack_pointer;
			running_task_index = task_index;
		} else {
			os_state = OS_RUNNING_IDLE;
			next_stack_pointer = idle_contex.stack_pointer;
		}
		break;
	case OS_RUNNING_TASK:
		task_list[running_task_index].stack_pointer = current_sp;
		if (task_list[running_task_index].state == TASK_RUNNING) {
			task_list[running_task_index].state = TASK_READY;
		}
		task_hit = search_next_task(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_RUNNING_TASK;
			task_list[task_index].state = TASK_RUNNING;
			next_stack_pointer = task_list[task_index].stack_pointer;
			running_task_index = task_index;
		} else {
			os_state = OS_RUNNING_IDLE;
			next_stack_pointer = idle_contex.stack_pointer;
		}
		break;
	case OS_RUNNING_IDLE:
		idle_contex.stack_pointer = current_sp;
		task_hit = search_next_task(&task_index);
		if (task_hit == TRUE) {
			os_state = OS_RUNNING_TASK;
			task_list[task_index].state = TASK_RUNNING;
			next_stack_pointer = task_list[task_index].stack_pointer;
			running_task_index = task_index;
		} else {
			os_state = OS_RUNNING_IDLE;
			next_stack_pointer = idle_contex.stack_pointer;
		}
		break;
	default:
		os_error_hook();
		break;
	}
	gpioToggle(GPIO3);
	enable_sys_tick_irq();
	return next_stack_pointer;
}

bool_t search_next_task(uint32_t* task_index) {
	bool_t task_hit = FALSE;
	uint32_t i;
	uint32_t j;
	uint32_t queue_length;
	uint32_t aux_index;

	for (i = 0; i < MAX_PRIORITY_QUEUE; i++) {
		if (task_stack_is_empty(&priority_queue[i]) == FALSE) {
			queue_length=task_stack_get_size(&priority_queue[i]);
			for (j = 0; j < queue_length ; j++) {
				task_stack_pop(&priority_queue[i], &aux_index);
				//Hago un switch con las tareas
				switch (task_list[aux_index].state) {
				case TASK_READY:
				case TASK_RUNNING:
					//Encontre mi tarea!
					(*task_index) = aux_index;
					//La pongo en la cola de prioridades ya que esta ready
					task_stack_push(&priority_queue[i], aux_index);
					return TRUE;
					break;
				case TASK_SLEEPING:
					//La tarea que este en sleeping directamente se saca porque no se usa
					break;
				default:
					//Nunca deberia haber una tarea que cumpla con este enum
					os_error_hook();
					break;
				}
			}
		}
	}
	return FALSE;
}

void do_scheduler(void) {
	__ISB();
	__DSB();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void init_task_stack(uint32_t stack[], uint32_t stack_size_bytes,
    uint32_t *stack_pointer, task_type_f entry_point, void * arg) {

	bzero(stack, stack_size_bytes);

	stack[stack_size_bytes / 4 - 1] = 1 << 24; /*XPSR.T = 1*/
	stack[stack_size_bytes / 4 - 2] = (uint32_t) entry_point; /*PC*/
	stack[stack_size_bytes / 4 - 3] = (uint32_t) task_return_hook; /*LR*/
	stack[stack_size_bytes / 4 - 8] = (uint32_t) arg; /*R0*/
	stack[stack_size_bytes / 4 - 9] = 0xFFFFFFF9; /*LR IRQ*/

	*stack_pointer = (uint32_t) &(stack[stack_size_bytes / 4 - 17]); //Corri el stack point 8 lugares hacia donde crece la pila

}

