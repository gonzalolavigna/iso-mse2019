#include "os.h"
#include "os_delay.h"
#include <stdint.h>
#include "string.h"
#include "sapi.h"
#include "task_stack.h"


//Numero maximo de las colas de prioridades que admite nuestro OS.
//Esto se saca en tiempo de compilacion para crear las colas, hay tantas colas
//como prioridades menos la IDLE
#define MAX_PRIORITY_QUEUE (sizeof(task_priority_t)-1)
//Ponemos un tamaño minimo del stack sin este no podemos crear un stack, un stack muy chicos
//Haria que se chocaría con el que viene
#define STACK_MIN_SIZE 20
//Este es el stack asignado al contexto idle-> TODO:Podría ser menor
#define IDLE_TASK_SIZE_BYTES 512

/*Variable globales del OS*/
/*Contexto de las tareas, es un arreglo que se direcciona de acuerdo a la tarea
 * actual que se esta ejecutando*/
task_context_t task_list[MAX_TASK_COUNT];
/*El contexto idle es igual a cualquier otra contexto pero tiene un tratamiento especial
 * Siempre esta en READY, y en la FSM del OS tiene un estado de entrada particulare si venimos
 * de correr el contexto idle*/
task_context_t idle_contex;

/*TODO:task_stack es un poco confuso*/
/*En esta cola solo se ponen los indices de las tareas*/
task_stack_t priority_queue[MAX_PRIORITY_QUEUE];

/*Cada vez que se crea un tarea se aumenta es contador*/
uint32_t task_count = 0;
/*Esta es una variable de estado indicando si estamos ejecutando tarea IDLE
 * Fndamental para hacer el cambio de contexto*/
uint32_t running_task_index = 0;
/*EL contexto de la tarea idle es interno al OS*/
uint32_t idle_task_stack[IDLE_TASK_SIZE_BYTES / 4];
/*El OS arranca siempre en OS_INIT, con os_init() con el primer pendsv se va a OS_TASK o OS_IDLE
 * Una vez que arranca nunca mas vuelve a OS_INIT*/
os_state_t os_state = OS_INIT;


/*Esta funcion es de la clase de Pablo Ridolfi, solo se llama desde adentro de init task para encapsularla*/
void init_task_stack(uint32_t stack[], uint32_t stack_size_bytes,
    uint32_t *stack_pointer, task_type_f entry_point, void * arg);
/*TODO: Esta es la funcion idle task que creo que habría que sacarla con el atribute waek */
void* idle_task(void * arg);

/*Por el momento todas las tareas vuelven al mismo lugar, no estamos dando margen a implementar algo*/
void task_return_hook(void * ret_val);
/*Esta funcion recorre las colas de prioridades, limpia las tareas en sleeping y hace el round robin*/
bool_t search_next_task(uint32_t* task_index);

/*Declaracion de funciones externas utilizadas por el SO*/
/*HAY QUE LLAMAR ESTA FUNCIÖN antes de hacer un os_init()*/
bool_t os_queue_init(void) {
	uint32_t i;
	i = MAX_PRIORITY_QUEUE;
	/*Inicializa todas las colas en los valores iniciales para evitar que colpse el S.O*/
	for (i = 0; i < MAX_PRIORITY_QUEUE; i++) {
		task_stack_init(&priority_queue[i]);
	}
	return TRUE;
}

bool_t os_task_create(uint32_t stack[], uint32_t stack_size_bytes,
    task_type_f entry_point, task_priority_t priority, void * arg) {

	//Reviso que no tenga mas tareas de las que puedo manejar.
	//Reviso que el stack que me estan pasando no sea menor a 20 porque sino no puedo crear un stack para el tamaño de los registros
	//Reviso que el stack sea divisible por 4, ya que las cuentas se hacen con ese multiplo.No quiero imaginar que puede pasar si pasa
	//Esto
	if (task_count >= MAX_TASK_COUNT || stack_size_bytes < 20
	    || stack_size_bytes % 4)
		return FALSE;
	/*Todas las tareas arrancan a READY lista para ejecutarse*/
	task_list[task_count].state = TASK_READY;
	/*El usuario configura a que cora va a ir parar un tarea*/
	task_list[task_count].priority = priority;
	/*Variable para indicar cuanto falta para habilitarse, utilizado con os_delay()*/
	task_list[task_count].reamaining_ticks = 0;
	/*Guardo el stack inicial a donde esta, capaz puede servir para reiniciar una tarea
	 *TODO: Ver si estos dos valores tienen utilidad */
	task_list[task_count].initial_stack_pointer = stack;
	task_list[task_count].stack_size_bytes = stack_size_bytes;
	/*Indica el indice de la tarea dentro del arreglo de tareas, no usado pero puede servir para direccionamiento
	 * por punteros, actualmente indexamos las tareas con un uint32_t*/
	task_list[task_count].task_index = task_count;
	/*Cualquier tarea que arranque no espera ningun evento -> Esta variable swe declara porque cuando una tarea
	 * espera un evento no tiene que ser decrementado su contador de remaining ticks*/
	/*TODO: Si queremos un funcionamiento porque un evento expire su tiempo hay que empezar por aca*/
	task_list[task_count].event_waiting = FALSE;

	//Inicializo el stack y ya queda actualizado el stack pointer al lugar donde tengo el stack
	//para ejecutar la tarea
	init_task_stack(stack, stack_size_bytes, &task_list[task_count].stack_pointer,
	    entry_point, arg);

	//Pongo en la cola, el indice de la tarea que se esta creando
	/*Por ejemplo:Si hay 3 Tareas con HIGH_PRIORITY la cola va a inicializarse con 3 valores*/
	task_stack_push(&priority_queue[priority], task_count);
	//Incremento la cantidad de tarea en el scheduler
	task_count++;

	return TRUE;
}


bool_t os_init(void) {
	/*Configuracion de la placa*/
	boardConfig();
	SystemCoreClockUpdate();
	/*Configuramos la PENDSV con menor prioridad*/
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	/*sistick en 1 ms TODO:Averiguar que es SystemCoreClock creo que es el periodo*/
	SysTick_Config(SystemCoreClock / 1000);

	//Configuracion de la tarea IDLE
	idle_contex.state = TASK_READY;
	idle_contex.priority = IDLE_PRIORITY;
	idle_contex.reamaining_ticks = 0;
	idle_contex.initial_stack_pointer = idle_task_stack;
	idle_contex.stack_size_bytes = IDLE_TASK_SIZE_BYTES;
	idle_contex.task_index = 0xFFFFFFFF;
	//Por definicion la tarea idle no permite ningun evento
	idle_contex.event_waiting = FALSE;
	init_task_stack(idle_task_stack,
	IDLE_TASK_SIZE_BYTES, &idle_contex.stack_pointer, idle_task,
	    (void *) 0x99999999);

	/*GPIOS de debug para ver con Analizador Logico GPIO 3 -> GET NEXT CONTEXT GPIO 4 -> SYSTICK*/
	gpioInit(GPIO3, GPIO_OUTPUT);
	gpioInit(GPIO4, GPIO_OUTPUT);
	/*TODO:Evaluar si no es necesario hacer un primer cambio de contexto,creo que estamos dando el arranque con el primer Sistick*/
	return TRUE;
}

/*TODO: Pasar a definición con weak()*/
void* __attribute__((weak))idle_task(void * arg) {
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
	/*Si hay alguna tarea en READY esto es TRUE, si es FALSE implica que hay
	 * que hacer uso de la tarea IDLE*/
	bool_t 		task_hit = FALSE;
	/*Siempre devuelvo el stack de donde tengo que desempaquetar el stack*/
	uint32_t 	next_stack_pointer;
	/*El task index me sirve para direccionar el arreglo de todas las tareas task_list*/
	uint32_t 	task_index;

	gpioToggle(GPIO3);
	/*Esto viene porque no quiero que el Sistick me interrumpa en un cambio de contexto ya que los dos
	 * hacen uso de las variable de estado de las tareas y de las colas -> Y puedo tener overrride de
	 * datos TODO:Existe alguna mejor manera de hacer esto?*/
	disable_sys_tick_irq();
	switch (os_state) {
	case OS_INIT:
		/*En el primer cambio de contexto no guardo ningun stack y directamente */
		/*Me fijo si hay tareas en READY -> task_index devuvelvo el indice de la tarea
		 * encontrada para ejecutar*/
		task_hit = search_next_task(&task_index);
		/*Si encontre un tarea en READY la pongo en ejecución*/
		if (task_hit == TRUE) {
			/*EL O.S va a estar ejecutando un tarea*/
			os_state = OS_RUNNING_TASK;
			/*Actualizo el estado de las tarea que encontre para ejecutar*/
			task_list[task_index].state = TASK_RUNNING;
			next_stack_pointer = task_list[task_index].stack_pointer;
			/*Actualizo la tarea que esta ejecutando el O.S*/
			running_task_index = task_index;
		} else {
			/*Si todas estan SLEEPING directamente ejecuto la tarea IDLE*/
			os_state = OS_RUNNING_IDLE;
			next_stack_pointer = idle_contex.stack_pointer;
		}
		break;
	case OS_RUNNING_TASK:
		/*Guardo el stack pointer de la tarea que se esta ejecutando*/
		/*Recordar que en running_task_index esta la tarea que se esta ejecutando*/
		task_list[running_task_index].stack_pointer = current_sp;
		/*Un cambio de scheduling puede venir de una llamado a la api del O.S
		 * por lo tanto si un tarea esta en SLEEPING por ejemplo si llamo a un task delay
		 * estaria mal pasarla a READY sino que se queda en SLEEPING por eso esta este IF*/
		if (task_list[running_task_index].state == TASK_RUNNING) {
			task_list[running_task_index].state = TASK_READY;
		}
		/*Me fijo si hay tareas en READY -> task_index devuvelvo el indice de la tarea
		 * encontrada para ejecutar*/
		task_hit = search_next_task(&task_index);
		if (task_hit == TRUE) {
			/*El OS va a estar ejecutando un tarea por lo tanto cuando vuelve a entrar
			 * vuelve a pasar por este estado*/
			os_state = OS_RUNNING_TASK;
			/*La tarea escogida le actualizo sus parametro y tambien configuro para
			 * que la pendSV pueda cambiar a dicha tarea*/
			task_list[task_index].state = TASK_RUNNING;
			next_stack_pointer = task_list[task_index].stack_pointer;
			running_task_index = task_index;
		} else {
			/*Si estan todas SLEEPING hago un cambio de contexto a IDLE*/
			os_state = OS_RUNNING_IDLE;
			next_stack_pointer = idle_contex.stack_pointer;
		}
		break;
	case OS_RUNNING_IDLE:
		/*Si vengo de correr una tarea IDLE tengo que guardar su contexto
		 * Tener en cuenta que nunca cambiamos su estado porque la tarea IDLE no
		 * puede ponerse a dormir*/
		idle_contex.stack_pointer = current_sp;
		/*Me fijo si hay tareas en READY -> task_index devuvelvo el indice de la tarea
		 * encontrada para ejecutar*/
		task_hit = search_next_task(&task_index);
		if (task_hit == TRUE) {
			/*Si encontre alguna directamente el O.S pasa a ejecutar un tarea*/
			os_state = OS_RUNNING_TASK;
			/*Actualizo los parametros de la tarea elegida para correr*/
			task_list[task_index].state = TASK_RUNNING;
			/*Cargo el stack pointer para que la pendsv haga el desempaquetado de la tarea
			 * elegida*/
			next_stack_pointer = task_list[task_index].stack_pointer;
			/*Siempre tengo que guardar el indice de la tarea que se esta ejecutando*/
			running_task_index = task_index;
		} else {
			/*Si no encontre ninguna tarea tengo que pasar a correr devuelta un tarea IDLE*/
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

/*Busca la primera tarea en READY, si hay alguna la pone de vuelta en la cola
 * pero al final de la misma, asi hago el round robin.
 * A medida que recorre las colas las tareas que estan durmiendo las saca de las colas
 * PAra que una tarea vuelva a la cola tiene que ser pueda por:
 * Vencimiento del daly la pone en la cola por atrás
 * Le dieron un evento
 * Recordar que el push los hacen las API del O.S*/

bool_t search_next_task(uint32_t* task_index) {
	bool_t task_hit = FALSE;
	/*Indice para recorrer las colas de prioridades*/
	uint32_t i;
	/*Indice para recorrer los indices de tareas dentro de la
	 * la cola de prioridades*/
	uint32_t j;
	/*Siempre tengo que contar hasta la cantidad de elementos
	 * que tenga guardado una cola*/
	uint32_t queue_length;
	uint32_t aux_index;

	/*Recorro todas las colas de prioridades*/
	for (i = 0; i < MAX_PRIORITY_QUEUE; i++) {
		/*Si la cola esta vacia directamente me voy a la proxima cola de
		 * prioridades*/
		if (task_stack_is_empty(&priority_queue[i]) == FALSE) {
			/*Saco cuantas tareas hay en la cola de tareas*/
			queue_length=task_stack_get_size(&priority_queue[i]);
			/*Recorro las tareas dentro de la cola de prioridades*/
			for (j = 0; j < queue_length ; j++) {
				/*Me llevo la tarea que esta adelante de todas*/
				task_stack_pop(&priority_queue[i], &aux_index);
				/*Hago un switch con su estado*/
				switch (task_list[aux_index].state) {
				case TASK_READY:
				case TASK_RUNNING:
					//Encontre mi tarea!
					(*task_index) = aux_index;
					//La pongo al final de la cola para hacer el round robin.
					//Si quisiera hace un esquema en el cual deba la tarea dar cooperativamente
					//Debería hacer un push_front --> A implementar
					task_stack_push(&priority_queue[i], aux_index);
					/*La tarea que encontre direcmente me hace volver
					 * El criterio es devolver al O.S ni bien se encuentra*/
					return TRUE;
					break;
				case TASK_SLEEPING:
					//La tarea que este en sleeping directamente se saca de la cola
					//Porque recordar que en el principio del for se hizo un pop.
					break;
				default:
					//Nunca deberia haber una tarea que no este en un estado conocido
					//En dicho caso me voy a un for infinito para el debugging
					os_error_hook();
					break;
				}
			}
		}
	}
	return FALSE;
}

/*Con esta tarea se fuerza un cambio de contexto del O.S*/
/*Usada por el Sistcik cada Sistick ocasiona una reevaluación de que tarea sigue
 * Tambien se hace con las APIs del OS que pasan una tarea a SLEEPING
 * No tiene sentido quedarse esperando hasta que llegue el Sistick*/
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

void os_enter_critical(void){
	__disable_irq();
}

void os_quit_critical(void){
	__enable_irq();
}

