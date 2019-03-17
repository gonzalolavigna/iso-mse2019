	.syntax unified
	.global SysTick_Handler
	.extern stack_1_pointer
	.extern get_next_context
	.text
	.thumb_func

	SysTick_Handler:
		#push {r4-r11,lr} 			/**Siempre guardar el link register**/
        #
		#mrs r0,msp
		#bl get_next_context
		#msr msp,r0 				 /** msp = r0 *//*Valor devvuelto por get_next_context*/
        #
		#pop {r4-r11,lr}
		push {r4-r11}
		/* msp = sp1 */
		ldr r0,=stack_1_pointer
		ldr r0,[r0]
		msr msp,r0

		pop  {r4-r11}

	return:
		bx lr
