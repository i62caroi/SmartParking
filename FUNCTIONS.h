#ifndef FUNCTIONS_h
#define FUNCTIONS_h

#include "VECTOR.h" // Operaciones vectoriales

/*  BATERÍA  */
#define ADC_BATTERY  (32u)
float battery_voltage = 3.9; 
bool sendBattery = false;   // Flag que indica la petición del estado de batería
float voltage;
int batteryPercentage;

/*----------------------------------------------------------------------------------*/
/*  FLAG PARA TOMAR VARIABLES DE REFERENCIA                                         */
/*----------------------------------------------------------------------------------*/
bool takeRef = true;        //  TRUE ==> tomar valores de referencia (plaza libre)
                            //  FALSE ==> no tomar valores de referencia 
                    
/*----------------------------------------------------------------------------------*/
/*  VARIABLES DEL ESTADO DE LA PLAZA                                                */
/*----------------------------------------------------------------------------------*/
int statusCar;              // 0 ==> libre
                            // 1 ==> ocupado 
                    
bool changedStatus;          //  TRUE ==> ha habido cambio de estado 
                            //  FALSE ==> no ha habido cambio de estado 
                    
/*----------------------------------------------------------------------------------*/
/*  FLAG PARA ENVIAR MENSAJE AL CAMBIAR DE ESTADO                                   */
/*----------------------------------------------------------------------------------*/
bool atChange = true;       //  TRUE ==> enviar solo al cambiar el estado 
                            //  FALSE ==> enviar cada 'nMin' minutos 

/*----------------------------------------------------------------------------------*/
/*  VARIABLES UMBRALES DE COMPROBACIÓN DE ESTADO                                    */
/*----------------------------------------------------------------------------------*/
float diffModule = 8.82;    // Umbral módulo (uT)
float gradesVectors = 4.65; // Umbral ángulo entre vectores (grados)
float angleVectors;         // Ángulo que forman el vector de referencia y el actual

/*----------------------------------------------------------------------------------*/
/*  VARIABLES PARA TIPO DE MENSAJE Y PERÍODO DE ENVÍO                               */
/*----------------------------------------------------------------------------------*/
int typeMesg;               // Tipo de mensaje DOWNLINK
int nMin = 1;                // Cada cuántos minutos enviar mensaje 
   




/***********************************************************************************/
/*------------------    FUNCIONES AUXILIARES    -----------------------------------*/
/***********************************************************************************/

/*---------------------------------------------------------------------------------------------------------
   isOcupied(): Función para conocer el estado de la plaza. 
                Si la variación del vector magnético actual respecto al de referencia está dentro de los
                umbrales establecidos, la plaza está ocupada.
----------------------------------------------------------------------------------------------------------*/
bool isOcupied(){
    if((abs(vect_free.module - vect_actual.module) >= diffModule) or (angleVectors >= gradesVectors))
          return true; 
}




/*----------------------------------------------------------------------------------------------------------
   getInfoMesg(): Función para obtener la información del mensaje DOWNLINK según el tipo.
                  Parámetro: 
                        val[] - Vector con el mensaje procesado en receiveMsg()
-----------------------------------------------------------------------------------------------------------*/
/*  --------------------------------------------------------------------------------------------------------
 *  | TIPO DE MENSAJE |            ESTRUCTURA          |             SIGNIFICADO                           | 
 *  --------------------------------------------------------------------------------------------------------
 *  |       1         |  "1"                           |  Tomar valores de referencia                      |
 *  |       2         |  "2-diffModule-gradesVectors"  |  Modificar umbrales de comparación                | 
 *  |       3         |  "3"                           |  Enviar estado de batería                         |     
 *  |    4 (opt 1)    |  "4-opcion-nMin"               |  Modificar período envío: cada 'nMin' minutos     |
 *  |    4 (opt 2)    |  "4-opcion"                    |  Modificar período envío: al cambiar estado plaza |
 *  --------------------------------------------------------------------------------------------------------
*/
void getInfoMesg(String val[]){
    typeMesg = val[0].toInt();    // Tipo de mensaje DOWNLINK recibido
    switch (typeMesg){
        case 1:  takeRef = true;                                                        break;
        case 2:  diffModule = val[1].toFloat(); gradesVectors = val[2].toFloat();       break;
        case 3:  sendBattery = true;                                                    break;
        case 4:  
            int option = val[1].toInt(); 
            switch (option){
                case 1: atChange = false; nMin = val[2].toInt();                        break;
                case 2: atChange = true;                                                break;
            }
                                                                                        break;
    }
}




/*---------------------------------------------------------------------------------------------------------
   statusBattery(): Función para tomar el estado de la batería
                    Parámetros: 
                            voltage             -  voltaje actual de la batería
                            batteryPercentage   -  porcentaje actual de la batería
----------------------------------------------------------------------------------------------------------*/
void statusBattery(float &voltage, int &batteryPercentage){
    analogReadResolution(10);
    analogReference(AR_INTERNAL1V0);
    int batteryLevel = analogRead(ADC_BATTERY);
    voltage = batteryLevel * (battery_voltage / 1023.0);
    batteryPercentage = ((voltage * 100) / battery_voltage);
    Serial.print("Voltage: "); Serial.print(voltage);
    Serial.print("    Batery Percentage: "); Serial.print(batteryPercentage); Serial.println("%\n");
}


#endif









