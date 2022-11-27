/*---------- LIBRERÍAS -----------*/
#include <Adafruit_HMC5883_U.h>     // Magnetometro
#include <Adafruit_Sensor.h>        // Tipo de sensor
#include <MKRWAN.h>                 // Modem LoRa y envío/recepción mensajes
#include <CayenneLPP.h>             // Cayenne myDevices ==> formato mensajes
#include <Wire.h>
#include <ArduinoLowPower.h>        // Dormir y reducir consumo


/*----- FICHEROS AUXILIARES ------*/
#include "VECTOR.h"                 // Operaciones vectoriales
#include "FUNCTIONS.h"              // Funciones auxiliares
#include "arduino_secrets.h"        // Claves aplicación en TTN

/*-------- CLAVES TTN -----------*/
String appEui = SECRET_APP_EUI;     // ID de la aplicación
String appKey = SECRET_APP_KEY;     // Clave de la aplicación para este dispositivo

/*  OBJETOS UTILIZADOS: magnetómetro, modem LoRa y Cayenne (formato mensajes) */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // Magnetometer con ID '12345'
LoRaModem modem;                    // Modem Lora para envío/recepción mensajes
CayenneLPP lpp(51);                 // Objeto CayenneLPP para codificación de mensajes

/* VARIABLES DE TIEMPO: Utilizadas para saber si es el momento de enviar el mensaje */
//unsigned long previousMillis = 0;        
//const long interval = 60000;   
/*
  MILLIS NO FUNCIONA CON SLEEP. SE DEBE CAMBIAR POR UN CONTADOR MANUAL.
  SABIENDO QUE SE LEE SIEMPRE CADA 1 MINUTO. PONER UN CONTADOR QUE EN CADA
  VUELTA DEL BUCLE SE INCREMENTE HASTA LLEGAR AL VALOR DE NMIN INDICADO.
  ES MUY RUDIMENTARIO, PERO SALVA EL PROBLEMA DE MILLIS.

*/
int minutes = 0;


void setup(){
    Serial.begin(9600);
    
    /*----- HMC5883L (MAGNETÓMETRO) ------*/
    mag.begin();
  
    /*----- CONEXIÓN LORAWAN --------------*/
    modem.begin(EU868);             // Establecer frecuencia de la región europea
    delay(1000); 
    modem.joinOTAA(appEui, appKey); // Conexión a una aplicación de TTN mediante sus claves
    modem.minPollInterval(60); 
} 



void loop(){

    /* Tomamos el tiempo actual para, en el caso en que se vaya a enviar
       mensaje cada 'nMin' minutos, saber si es el momento de enviarlo */
    //unsigned long currentMillis = millis();
    //CAMBIAR MILLIS POR CONTADOR RUDIMENTARIO

    /*-------------------------------------------------*/
    /*---------- HMC5883L (MAGNETÓMETRO)  -------------*/
    /*-------------------------------------------------*/
    sensors_event_t event;  // Nuevo evento del magnetómetro
    mag.getEvent(&event);   // Obtención de valores del campo magnético (uT)
    

    /*-------------------------------------------------*/
    /*------------- VALORES DE REFERENCIA -------------*/
    /*-------------------------------------------------*/
    if(takeRef){ /* Esta flag indica si tomar valores de referencia.
                  Puede manipularse mediante un mensaje DOWNLINK de tipo 1. */
        vect_free = {event.magnetic.x, event.magnetic.y, event.magnetic.z, -100.00}; 
        getModule(vect_free); 
        takeRef = false; /* Reestablecer flag de referencia para no volver a tomar los valores
                          hasta que se indique */
    }

    /*-------------------------------------------------*/
    /*-------------- VALORES ACTUALES -----------------*/
    /*-------------------------------------------------*/
    vect_actual = {event.magnetic.x, event.magnetic.y, event.magnetic.z, -100.00}; 
    getModule(vect_actual); 
    
    Serial.print("\n\n Intensidad de referencia: "); Serial.print(vect_free.module);   Serial.print(" uT");
    Serial.print("\n\n Intensidad actual: ");        Serial.print(vect_actual.module); Serial.print(" uT");
    Serial.println("\n");


    /*-------------------------------------------------*/
    /*------------- ÁNGULO ENTRE VECTORES -------------*/
    /*-------------------------------------------------*/
    /*   Tomar ángulo formado entre el vector de referencia y el actual */
    angleVectors = getAngleVectors(vect_free, vect_actual);
    Serial.print("\nÁngulo entre vectores: ");       Serial.println(angleVectors);


    /*-------------------------------------------------*/
    /*----------  ACTUALIZACIÓN ESTADO PLAZA ----------*/
    /*-------------------------------------------------*/
    /*
     *  A partir de los valores de referencia y los umbrales establecidos,
     *  se comparan con los valores actuales. Si se encuentran dentro de
     *  los umbrales, es decir, si se cumple isOcupied(), significa que
     *  la plaza está ocupada.
     *  Estos umbrales se han establecido universalmente de tal manera
     *  que no importe la forma en que diferentes vehículos distorsionen el
     *  campo magnético.
     *  
     *  Además, solo se cambia el estado de la plaza cuando es diferente
     *  al estado anterior.
     *
    */
    if(isOcupied() and (statusCar == 0)){ // Si está ocupado, pero estaba libre 
        statusCar = 1; 
        changedStatus = true; // Se ha cambiado el estado: libre --> ocupado 
    }
    else if(!isOcupied() and (statusCar == 1)){ // Si está libre, pero estaba ocupado 
        statusCar = 0;
        changedStatus = true; // Se ha cambiado el estado: ocupado --> libre 
    }
    Serial.print("\nEstado plaza (0: libre    1: ocupado): "); Serial.println(statusCar);


    
    
    /*-------------------------------------------------*/
    /*-------------- ENVÍO DEL MENSAJE ----------------*/
    /*-------------------------------------------------*/
    // Para ahorrar energía, solo se da formato al mensaje cuando se vaya a realizar el envío
    if(atChange and changedStatus){ /*  Si se quiere enviar solo al cambiar estado 
                                       y ha habido cambio */
        getCayenneFormat(); 
        sendMsg();
        changedStatus = false; // Se reestablece la flag del cambio de estado
    }
    else if(!atChange){ /* Si no se quiere enviar mensaje al cambiar de estado,
                           sino cada 'nMin' minutos.
                           La situación del campo magnético se lee cada
                           minuto, pero se envía según indique esta flag. */
        /*if ((currentMillis - previousMillis) >= (interval * nMin)) { //
            previousMillis = currentMillis;
            getCayenneFormat();
            sendMsg();
        }*/
        //SE DEBE CAMBIAR MILLIS POR CONTADOR RUDIMENTARIO SABIENDO QUE SE LEE CADA 1 MIN
        if(minutes != nMin) minutes++;
        else{
            getCayenneFormat();
            sendMsg();
            minutes = 0;
        }
    }
  

    /*-------------------------------------------------*/
    /*------------ RECEPCIÓN DEL MENSAJE --------------*/
    /*-------------------------------------------------*/
    receiveMsg();
 

    /*  ESPERA BLOQUEANTE */
    //delay(interval); //Se obtiene vector magnético cada minuto, pero se envía cuando se indique
    LowPower.sleep(interval);
    //LowPower.deepSleep(interval);

} // Fin función 'loop()'




/*---------------------------------------------------------------------------------------------------------
   getCayenneFormat(): Función para codificar con CayenneLPP el mensaje a enviar. 
----------------------------------------------------------------------------------------------------------*/
void getCayenneFormat(){
    lpp.reset(); // Se resetea el objeto CayenneLPP
    /*  Se añade al mensaje como valor digital:
     *      - Estado de la plaza (0: libre  1: ocupado)
     *  Por otro lado, si se han solicitado, se añaden como valores analógicos: 
     *      - Voltaje de la batería 
     *      - Porcentaje remanente de batería 
    */
    /* Estos valores se enviaban en las pruebas. En la versión final solo debe enviar el estado.
          lpp.addAnalogInput(1, vect_free.module);
          lpp.addAnalogInput(2, vect_actual.module);
          lpp.addAnalogInput(3, angleVectors);
    */
    lpp.addDigitalInput(4, statusCar);
    if(sendBattery){ /* Si se ha pedido el estado de la batería mediante un mensaje
                        DOWNLINK de tipo 3, se añade al mensaje UPLINK a enviar. */
        statusBattery(voltage, batteryPercentage);
        lpp.addAnalogInput(5, voltage);
        lpp.addAnalogInput(6, batteryPercentage);
        sendBattery = false; // Se reestablece la flag que pedía el estado de la batería
    }
}



/*---------------------------------------------------------------------------------------------------------
   sendMsg(): Función para enviar el mensaje previamente codificado con CayenneLPP. 
----------------------------------------------------------------------------------------------------------*/
void sendMsg(){
    int err;
    modem.beginPacket();
    modem.write(lpp.getBuffer(), lpp.getSize()); // Enviar mensaje UPLINK codificado en formato CayenneLPP
    err = modem.endPacket(true); 
    if (err > 0) {
      Serial.println("\nMessage sent correctly!");
    } 
    else{
      Serial.println("\nError sending message :(\n");
    }
}



/*---------------------------------------------------------------------------------------------------------
   receiveMsg(): Función en la que se comprueba si se ha recibido mensaje DOWNLINK y, si es así, se
                 procesa su información. Posteriormente, en getInfoMesg(), se actúa acorde a los datos
                 recibidos. 
----------------------------------------------------------------------------------------------------------*/
void receiveMsg(){
  if (modem.available()) { //Si se ha recibido mensaje DOWNLINK
      /*  Se lee la cadena recibida */
      char mesgRcv[64]; //Hasta 64 caracteres recibidos
      int i = 0;
      while (modem.available()) {
        mesgRcv[i++] = (char)modem.read();
      }
      String strRcv = String(mesgRcv);
      strRcv.trim();
      Serial.print("Received: ");
      Serial.print(strRcv);

      /******* PROCESAMIENTO DEL MENSAJE DOWNLINK ******
       * Las siguientes líneas procesan el mensaje DOWNLINK recibido:
       *    Siguiendo el protocolo establecido, este mensaje debe enviar una cadena de valores separados 
       *    por un guión '-'. El primer valor es el tipo de mensaje y los demás, si se incluyen, dependen 
       *    de ese tipo de mensaje. 
      */
      String val[10]; // Se admiten hasta 10 valores 
      int nVal = 0, index;
      while (strRcv.length() > 0){
          index = strRcv.indexOf('-');
          if (index == -1){ 
            val[nVal++] = strRcv; 
            break;
          }
          else{
            val[nVal++] = strRcv.substring(0, index); 
            strRcv = strRcv.substring(index+1); 
          }
      }

      // Tras procesar el mensaje DOWNLINK, el vector 'val[]' contiene los valores recibidos. 
      getInfoMesg(val);   // Acciones según tipo de mensaje
  }    
}
