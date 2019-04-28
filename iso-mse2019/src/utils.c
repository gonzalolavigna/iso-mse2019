#include <stdint.h>
#include <stdio.h>
#include "sapi.h"
#include "utils.h"
#include "math.h"

bool_t convert_float_to_str(uint8_t * str, float number, uint8_t decimal_points){
	//Este buffer es para guardar cuantos ceros despues de la coma obtener
	uint8_t  temp_buffer[MAX_DECIMAL_POINTS+1];
	//Iterador para poder contar en los distintos procesos de esta funcion.
	uint32_t i;
	//Aca se guarda la parete entera positiva del numbero en punto flotante
	uint32_t integer_part;
	//Aca se guarda en enteros la parte decimal del numero en punto flotante
	uint32_t integer_decimal_part;
	//En el medio hay que hacer algunos casteos
	uint32_t temp;
	//Esta es la cantidad de ceros despues de la coma.
	uint32_t number_of_zeros = 0;
	//Aca guardamos en float la parte decimal
	float 	 decimal_part;
	//Operamos sobre este numero que siempre lo tenemos y es positivo
	float 	 temp_number;
	//Esto se usa como caracter para tener el signo
	uint8_t  sign;

	//Por defecto el string que guarda cuantos ceros despues de la coma se llena de caracteres no
	//imprimibles
	for(i=0;i<MAX_DECIMAL_POINTS+1;i++){
		temp_buffer[i] = 0;
	}

	//Esta funcion la idea es no irse de alcance y si pasamos cierta cantidad de numeros decimales
	//devolvemos un falta
	if(decimal_points > MAX_DECIMAL_POINTS){
		return FALSE;
	}
	//Si es negativo primero se invierte para trabajar en positivo
	if(number < 0){
		temp_number = (-1.0)*number;
		//Obtengo la parte entera haciendo un casteo.
		integer_part = (uint32_t) temp_number;
		//La parte decimal es el numero recibido menos la parte entera. (Recordar que los hicimos positivo)
		decimal_part = temp_number - (float)integer_part;
		//Aca multiplicamos por 10 dependiendo de cuantos numeros decimales queremos
		for(i = 0; i < decimal_points;i++){
			//Lo multiplico tanta veces como necesite para obtener el numero en la presicion que quiero
			decimal_part *= 10;
			//Casteo la parte decimal para ver si es cero, porque si lo es hgay que agregar ceros despues de la coma
			temp = (uint32_t)decimal_part;
			if(temp == 0){
				//Esta es la cantidad de ceros que hay que agregar
				number_of_zeros++;
			}
		}
		//Una vez que hicimos todas las multiplicaciones agregamos la cantidad de ceros despues de la coma.
		for(i=0;i<number_of_zeros;i++){
			temp_buffer[i] = '0';
		}
		//Me llevo la parte decimal casteando
		integer_decimal_part = (uint32_t) decimal_part;
		//Esto es la concatenacion de la parte entera + el '.' + la cantidad de ceros despues de la coma + la parte decimal
		//Si despues de multiplicar n veces tenemos igual a cero no hay parte decimal que imprimir
		if(integer_decimal_part != 0){
			sprintf(str,"-%d.%s%d",integer_part,temp_buffer,integer_decimal_part);
		}else {
			sprintf(str,"-%d.%s",integer_part,temp_buffer);
		}

	}else {
		temp_number = number;
		//Obtengo la parte entera haciendo un casteo.
		integer_part = (uint32_t) temp_number;
		//La parte decimal es el numero menos la parte entera.
		decimal_part = temp_number - (float)integer_part;
		//Aca multiplicamos por 10 dependiendo de cuantos numeros decimales queremos
		for(i = 0; i < decimal_points;i++){
			//Lo multiplico tanta veces como necesite para obtener el numero en la precision que quiero.
			decimal_part *= 10;
			//Casteo la parte decimal para ver si es cero, porque si lo es hgay que agregar ceros despues de la coma
			temp = (uint32_t)decimal_part;
			if(temp == 0){
				//Esta es la cantidad de ceros que hay que agregar
				number_of_zeros++;
			}
		}
		//Una vez que hicimos todas las multiplicaciones agregamos la cantidad de ceros despues de la coma.
		for(i=0;i<number_of_zeros;i++){
			temp_buffer[i] = '0';
		}
		//Me llevo la parte decimal casteando
		integer_decimal_part = (uint32_t) decimal_part;
		//Esto es la concatenacion de la parte entera + el '.' + la cantidad de ceros despues de la coma + la parte decimal
		//Si despues de multiplicar n veces tenemos igual a cero no hay parte decimal que imprimir
		if(integer_decimal_part != 0){
			sprintf(str,"%d.%s%d",integer_part,temp_buffer,integer_decimal_part);
		} else {
			sprintf(str,"%d.%s",integer_part,temp_buffer,integer_decimal_part);
		}
	}
	return TRUE;
}
