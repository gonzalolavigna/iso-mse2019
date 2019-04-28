	.syntax unified
	.text
	.global PendSV_Handler
	.extern get_next_context
	.thumb_func

	PendSV_Handler:
								/*Contexto FPU*/
		tst lr,0x10 			/* lr & 0x10 (Comparacion es una and bit a bit)*/
		it 	eq 					/**Si el resultado da igual a 0*/
		vpusheq {s16-s31} 		/**Pusheo S16 y S31 a la pila y fuerzo stacking s0-s15*/

		push {r4-r11,lr} 		/**Siempre guardar el link register**/

		mrs r0,msp
		bl get_next_context
		msr msp,r0 				/** msp = r0 *//*Valor devuelto por get_next_context*/

		pop {r4-r11,lr}

		tst lr,0x10
		it eq
		vpopeq {s16-s31}

	return:
		bx lr
