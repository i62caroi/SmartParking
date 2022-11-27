var SHEET_NAME    =   "last_data";            // Hoja de cálculo
var DEV_EUI_1     =   "A8610A33344B7115";     // Dispositivo plaza 1
var DEV_EUI_2     =   "A8610A33343E7215";     // Dispositivo plaza 2
var N_DEVICES     =   10;                     // Número de dispositivos utilizados (2 reales y 8 ficticios)




/* ---------------------------------------------------------------------------------------
  doPost(): Función que se activa cuando la aplicación web recibe una solicitud POST.
            Obtiene la información del mensaje recibido en la solicitud POST y actualiza
            los datos.
            Parámetros:
                  e - parámetro de evento que contiene información de la solicitud
 --------------------------------------------------------------------------------------- */
function doPost(e) {
  var myData        =   JSON.parse(e.postData.contents); // Mensaje enviado por TTN
  var deviceValues  =   getDeviceValues(myData);         // Obtiene algunos datos del mensaje

  updateDeviceValues(deviceValues);                      // Actualiza la información
  
  return HtmlService.createHtmlOutput("post request received");
}



/* ---------------------------------------------------------------------------------------
  getDeviceValues(): Función que obtiene determinada información del mensaje recibido 
                     Parámetros:
                        myData - mensaje JSON incluido en la petición POST recibida
 --------------------------------------------------------------------------------------- */
function getDeviceValues(myData){
  var dev_eui      =   myData.end_device_ids.dev_eui;                        // DEV_EUI
  if (dev_eui  == DEV_EUI_1) {       // Mensaje enviado por el dispositivo 1
    var device     =   "Plaza 1";                                            // ID Plaza
  }
  else if (dev_eui == DEV_EUI_2) {  // Mensaje enviado por el dispositivo 2
    var device     =   "Plaza 2";                                            
  }
  var modFree      =   myData.uplink_message.decoded_payload.analog_in_1;    // Módulo referencia
  var modNew       =   myData.uplink_message.decoded_payload.analog_in_2;    // Módulo actual
  var angle        =   myData.uplink_message.decoded_payload.analog_in_3;    // Ángulo
  var status       =   myData.uplink_message.decoded_payload.digital_in_4;   // Estado plaza
  var latitude     =   myData.uplink_message.locations.user.latitude;        // Latitud (ubicación)
  var longitude    =   myData.uplink_message.locations.user.longitude;       // Longitud (ubicación)
  var address      =   latitude + ', ' + longitude;                          // Ubicación

  return { dev_eui, device, modFree, modNew, angle, status, latitude, longitude, address };
}





/* ---------------------------------------------------------------------------------------
  updateDeviceValues(): Función que actualiza la hoja de cálculo con la información del 
                        mensaje recibido. 
                        Parámetros:
                            deviceValues - determinados datos extraídos del mensaje
 --------------------------------------------------------------------------------------- */
function updateDeviceValues(deviceValues){
  //  HOJAS DE CÁLCULO UTILIZADAS
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(SHEET_NAME); // Última actualización

  //  VALORES A INSERTAR LOG
  // time | dev_eui | device | modFree | modNew | angle | status | address | temperature | humidity | wind | description 
  var time            =   Date();
  var weatherValues   =   getWeather(deviceValues.latitude, deviceValues.longitude);

  var info = [time,deviceValues.dev_eui,deviceValues.device,deviceValues.modFree, deviceValues.modNew,deviceValues.angle,deviceValues.status,deviceValues.address, weatherValues.temperature, weatherValues.humidity, weatherValues.wind, weatherValues.description];

  if (deviceValues.dev_eui == DEV_EUI_1) var row = 2;       // Fila del dispositivo 1
  else if (deviceValues.dev_eui == DEV_EUI_2) var row = 3;  // Fila del dispositivo 2
  for (let i = 0; i < info.length; i++) { 
    sheet.getRange(row, i + 1).setValue(info[i]);           // Actualizar fila
  }

  SpreadsheetApp.flush();                                   // Aplicar cambios
}




/* ---------------------------------------------------------------------------------------
  getWeather(): Función que obtiene la información del tiempo en una ubicación determinada 
                a partir de la API de OpenWeatherMap. 
                Parámetros:
                      latitude  - latitud de la ubicación
                      longitude - longitud de la ubicación
 --------------------------------------------------------------------------------------- */
function getWeather(latitude, longitude){
  // INFORMACIÓN PARA OBTENER EL TIEMPO:
  /*  lat     = latitud
      lon     = longitud
      API key = 774dcf442041de7dbb0de2d18e7cfb96 (https://openweathermap.org/api)
      lang    = es (idioma de la descripción)
      units   = metric
  */

  // Obtener el tiempo actual
  var url = "api.openweathermap.org/data/2.5/weather?lat=" +latitude+ "&lon=" +longitude+ "&appid=774dcf442041de7dbb0de2d18e7cfb96&lang=es&units=metric"; 
  var response = UrlFetchApp.fetch(url, {method: 'GET',headers: {'Content-Type': 'application/json','Accept': 'application/json'} });
  
  var weatherData     =   JSON.parse(response.getContentText()); // Información obtenida

  var temperature     =   weatherData.main.temp + "ºC";
  var humidity        =   weatherData.main.humidity + "%";
  var wind            =   ((parseFloat(weatherData.wind.speed)*3600)/1000).toString() + " km/h"; 
                                 // m/s ==> ((m/s)*3600)/1000 ==> km/h 
  var description     =   (weatherData.weather[0].description).charAt(0).toUpperCase() + (weatherData.weather[0].description).slice(1);       // Primera letra de la descripción a mayúscula

  return { temperature, humidity, wind, description };
  
}




/*  UBICACIONES DISPOSITIVOS:   SON NECESARIAS PARA ACTUALIZAR EL TIEMPO CADA 15 MIN
      - DEV_1 : 37.914947, -4.716554        - DEV_6 : 37.915811, -4.721464
      - DEV_2 : 37.914823, -4.725539        - DEV_7 : 37.915698, -4.719562
      - DEV_3 : 37.914705, -4.723086        - DEV_8 : 37.916005, -4.720535
      - DEV_4 : 37.914337, -4.722720        - DEV_9 : 37.916053, -4.721401
      - DEV_5 : 37.913834, -4.722756        - DEV_10 : 37.913326, -4.719191
 
    ORDEN EN VECTORES: DEV_1, DEV_2, DEV_3, DEV_4, DEV_5, DEV_6, DEV_7, DEV_8, DEV_9, DEV_10
*/
var latitudeVector      =   [37.914947, 37.914823, 37.914705, 37.914337, 37.913834, 37.915811, 37.915698, 37.916005, 37.916053, 37.913326];   
var longitudeVector     =   [-4.716554, -4.725539, -4.723086, -4.722720, -4.722756, -4.721464, -4.719562, -4.720535, -4.721401, -4.719191];   



/* ---------------------------------------------------------------------------------------
  updateExtraInfoAllDevices(): Función que actualiza la información del tiempo de todas
                               las plazas. Se activa cada 15 minutos mediante un trigger. 
 --------------------------------------------------------------------------------------- */
function updateExtraInfoAllDevices(){
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(SHEET_NAME); // Última actualización
  for (let i = 0; i < N_DEVICES; i++) { 
      var weatherValues = getWeather(latitudeVector[i], longitudeVector[i]); //Vectores globales con coordenadas
      var time = Date();
      var row = i + 2; // Comienza por fila 2 (device 1)
                       // COLUMNAS A MODIFICAR:   time(1), temperature(9), humidity(10), wind(11), description(12)   
      sheet.getRange(row, 1).setValue(time);  
      sheet.getRange(row, 9).setValue(weatherValues.temperature);  
      sheet.getRange(row, 10).setValue(weatherValues.humidity);  
      sheet.getRange(row, 11).setValue(weatherValues.wind);  
      sheet.getRange(row, 12).setValue(weatherValues.description);  
  }
  SpreadsheetApp.flush();
}
