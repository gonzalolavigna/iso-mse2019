	.syntax unified
	.text
	.global SysTick_Handler
	.extern get_next_context
	.thumb_func

	SysTick_Handler:
		push {r4-r11,lr} 		/**Siempre guardar el link register**/
		mrs r0,msp
		bl get_next_context
		msr msp,r0 				/** msp = r0 *//*Valor devuelto por get_next_context*/

		pop {r4-r11,lr}

	return:
		bx lr
