#ifndef __UTILS__
#define __UTILS__

#include <stdint.h>
#include "sapi.h"
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

//Nuestra funcion solo va a permitir generar 5 puntos decimales
#define MAX_DECIMAL_POINTS 5

bool_t convert_float_to_str(uint8_t * str, float number, uint8_t decimal_points);


#ifdef __cplusplus
}
#endif

#endif
