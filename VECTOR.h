#ifndef VECTOR_h
#define VECTOR_h

#include <Arduino.h>

/*-------------------------------------------------*/
/*    VALORES DEL VECTOR                           */
/*-------------------------------------------------*/
struct myVector{ 
  float x;        // Componente x
  float y;        // Componente y
  float z;        // Componente z
  float module;   // Módulo del vector
}vect_free, vect_actual;



/************************************************************************************/
/*--------------    FUNCIONES OPERACIONES VECTORIALES    ---------------------------*/
/************************************************************************************/

/*-----------------------------------------------------------------------------------
   getModule(): Función para obtener el módulo de un vector a partir de sus
                componentes.
                Parámetro: 
                        vect - objeto de tipo myVector que incluye las componentes
------------------------------------------------------------------------------------*/
void getModule(myVector &vect){
    vect.module = sqrt(pow(vect.x, 2) + pow(vect.y, 2) + pow(vect.z, 2));
}


/*-----------------------------------------------------------------------------------
   dotProduct(): Función para obtener el producto escalar entre dos vectores.
                 Parámetros: 
                        vect1 - myVector que representa al primer vector
                        vect2 - myVector que representa al segundo vector
------------------------------------------------------------------------------------*/
float dotProduct(myVector &vect1, myVector &vect2){
    float product = vect1.x * vect2.x + vect1.y * vect2.y + vect1.z * vect2.z;
    return product;
}
 

/*-----------------------------------------------------------------------------------
   getAngleVectors(): Función para obtener el ángulo formado entre dos vectores.
                      Parámetros: 
                            vect1 - myVector que representa al primer vector
                            vect2 - myVector que representa al segundo vector
------------------------------------------------------------------------------------*/
float getAngleVectors(myVector &vect1, myVector &vect2){
    float aux = (dotProduct(vect1, vect2))/(vect1.module * vect2.module);
    float angle = acos(aux) * 180.0 / PI;
    return angle;
}


#endif




