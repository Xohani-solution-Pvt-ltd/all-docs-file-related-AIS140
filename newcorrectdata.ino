// Define all library
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPIMemory.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>

// serial communication between flash and esp32

SPIFlash flash(5, &SPI);
MPU6050 mpu; //define gyro

SoftwareSerial gpsSerial(33,32);
//serial communication between simmodule and esp32

SoftwareSerial gprsSerial(17, 16);
#define portTICK_PERIOD_MS 1

// define required gyro values
TinyGPSPlus gps;
long lat, lon;
unsigned long lastInternetCheckTime = 0;
const unsigned long internetCheckInterval = 20000; 
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;
String api_Key1 = "FT5SA8SNM1MQAUHM";
String server1 = "GET https://api.thingspeak.com/update?api_key=";

// deine task name on different core

TaskHandle_t Task1;
TaskHandle_t Task2;

const size_t FLASH_SIZE = 2097152;  // Flash size in bytes
const size_t SECTOR_SIZE = 4096;  // Flash sector size in bytes


const int STACK_SIZE = 100; // Adjust the stack size as needed
unsigned long addressStack[STACK_SIZE];
int stackIndex = 0;

void pushToStack(unsigned long address) {
  if (stackIndex < STACK_SIZE) {
    addressStack[stackIndex++] = address;
  } else {
    // Stack is full, overwrite the oldest entry
    unsigned long oldestAddress = addressStack[0];
    for (int i = 0; i < STACK_SIZE - 1; ++i) {
      if (addressStack[i] < oldestAddress) {
        oldestAddress = addressStack[i];
      }
    }
    flash.eraseSector(oldestAddress);

    // Replace the oldest entry with the new address
    addressStack[0] = address;
  }
}

unsigned long popFromStack() {
  if (stackIndex > 0) {
    return addressStack[--stackIndex];
  } else {
    // Stack is empty
    return 0;
  }
} 

void setup() {
  Serial.begin(9600);
  flash.begin();
  gpsSerial.begin(9600); // connect gps sensor
  Wire.begin();
  mpu.initialize();
  gprsSerial.begin(9600);
  
  xTaskCreatePinnedToCore(send_real_time_data, "Task1", 10000, NULL, 1, &Task1, 0);    
  vTaskDelay(10000 / portTICK_PERIOD_MS);                    
  xTaskCreatePinnedToCore(send_stored_to_server, "Task2", 10000, NULL, 1, &Task2, 1);     
  vTaskDelay(10000 / portTICK_PERIOD_MS);   
}

void loop() {


  send_real_time_data(NULL);
  send_stored_to_server(NULL);
}


void internet_connect(){
  gprsSerial.println("AT+CPIN?");
  delay(1000);
  gprsSerial.println("AT+CREG?");
  delay(1000);
  gprsSerial.println("AT+CGATT?");
  delay(1000);
  gprsSerial.println("AT+CIPSHUT");
  delay(1000);
  gprsSerial.println("AT+CIPSTATUS");
  delay(2000);
  gprsSerial.println("AT+CIPMUX=0");
  delay(2000);  
  gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000);
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(1000);
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(1000);
  gprsSerial.println("AT+CIPSPRT=0");
  delay(1000);
}

String readSerialData() {
  String data = "";
  while (gprsSerial.available() != 0)
    data += (char)gprsSerial.read();
  delay(5000);
  return data;
}



bool checkInternetConnectivity() {
  gprsSerial.println("AT+CGATT?");
  delay(1000);
  String response = "";

  // Wait for a complete response
  while (gprsSerial.available()) {
    response += gprsSerial.readStringUntil('\n');
  }

 // Serial.println(response); // Print the complete response for debugging

  return response.indexOf("+CGATT: 1") != -1;  // Returns true if GPRS is attached (internet is available)
}


void gyro_data(){
   mpu.getRotation(&rawGyroX, &rawGyroY, &rawGyroZ);
   mpu.getAcceleration(&rawAcceloX, &rawAcceloY, &rawAcceloZ);
   rawTemp = mpu.getTemperature();

   AcceloX = rawAcceloX / 16384.0;
   AcceloY = rawAcceloY / 16384.0;
   AcceloZ = rawAcceloZ / 16384.0;

   gyroX = rawGyroX / 131.0;
   gyroY = rawGyroY / 131.0;
   gyroZ = rawGyroZ / 131.0;

   temp = ((rawTemp) / 340.0) + 36.53;
}
void GPS_data(){
  if (millis() > 50000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {   
      if (gps.date.isValid() && gps.date.year() <2024) // Check if date is valid
      {
        Serial.print("Date: ");
        Serial.print(gps.date.year()); Serial.print("/");
        Serial.print(gps.date.month()); Serial.print("/");
        Serial.println(gps.date.day());
      }
      else
      {
        Serial.println("Invalid date");
      }

      Serial.print("Hour: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second());

      Serial.print("GPS Fix: ");
      Serial.println(gps.satellites.isValid()  ? "1" : "0"); // Display GPS fix status as 1 or 0
       
      Serial.print("SATS: ");
      Serial.println(gps.satellites.value());
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");
      Serial.println(gps.location.lng(), 6);
       Serial.print(gps.location.lat() >= 0 ? "N" : "S"); // Latitude direction
      Serial.print(" | LONG: ");
      Serial.print(gps.location.lng(), 6);
      Serial.print(" ");
      Serial.println(gps.location.lng() >= 0 ? "E" : "W"); // Longitude direction
      Serial.print("ALT: ");
      Serial.println(gps.altitude.meters());
      Serial.print(" | HEADING: ");
      Serial.println(gps.course.deg()); // Heading in degrees
      Serial.print("SPEED: ");
      Serial.println(gps.speed.mps());
      Serial.println("---------------------------");
      delay(1000);
    }
  }
}


void sendStoredDataToServer(String dataToSend) {
  DynamicJsonDocument doc(512);

  // Deserialize the JSON string
  DeserializationError error = deserializeJson(doc, dataToSend);

  // Check if the deserialization was successful
  if (error) {
    Serial.print(F("Failed to deserialize JSON: "));
    Serial.println(error.c_str());
    return;
  }

  // Extract values from the JSON document
  float field1Value = doc["gyroX"];
  float field2Value = doc["gyroY"];
  float field3Value = doc["gyroZ"];
  float field4Value = doc["Temp"];

  // Call sendDataToThingsSpeak to send the extracted values
  sendDataToThingsSpeak("GET https://api.thingspeak.com/update?api_key=", "FT5SA8SNM1MQAUHM", field1Value, field2Value, field3Value, field4Value);
  Serial.println("stored data sent to internet");
}


void sendDataToThingsSpeak(String server, String api_Key, int field1, int field2, int field3, int field4){
  
   gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
   delay(4000);
   
   gprsSerial.println("AT+CIPSEND");
   delay(4000);
   
   String str = server + api_Key + "&field1=" +String (field1) + "&field2=" + String(field2) + "&field3=" +String(field3) + "&field4=" + String(field4);

  //  Serial.println(str);
   gprsSerial.println(str);
   
   gprsSerial.println((char)26);//sending
   delay(5000); 
   gprsSerial.println();
 
}

void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}

void send_real_time_data( void * pvParameters ){
   while(true) {

   gyro_data();
   unsigned long currentMillis = millis();
    
  //  Check internet connectivity every 20 seconds
   if (currentMillis - lastInternetCheckTime >= internetCheckInterval) {
     internet_connect();
     //Serial.println("Akash");
     lastInternetCheckTime = currentMillis;
   }

  if (checkInternetConnectivity()) {
    Serial.println("Internet is available");
    sendDataToThingsSpeak("GET https://api.thingspeak.com/update?api_key=", "FT5SA8SNM1MQAUHM", AcceloX, AcceloY, AcceloZ, temp);
  } else {
    Serial.println("Internet is not available");
  }
  //vTaskDelay(30000 / portTICK_PERIOD_MS);

  }
  }
void send_stored_to_server ( void * pvParameters ){
 while(true){
 Serial.print(F("Flash size: "));
 Serial.print((long)(flash.getCapacity() / 1000));
 Serial.println(F("Kb"));
 //flash.eraseChip();

  if (checkInternetConnectivity()) {
    
    Serial.println("Internet is available");
    int addressToSend = popFromStack();
      while (addressToSend != 0) {
        String dataToSend;
        if (flash.readStr(addressToSend, dataToSend)) {
          
          Serial.println(dataToSend);
          sendStoredDataToServer(dataToSend);
           //flash.eraseSector(addressToSend);

          Serial.println(F("Data sent from flash memory"));
  }
        // Get the next address from the stack
        addressToSend = popFromStack();
         if (addressToSend != 0) {
    flash.eraseSector(addressToSend);
  }
  }
  }
  else {
   Serial.println("Internet is not available");
  //  Serial.println("Data stored in flash memory");
   gyro_data();

   int strAddrSecondString = 0;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
   Serial.println();
 
   DynamicJsonDocument gyroDoc(512);
   gyroDoc["gyroX"] = gyroX;
   gyroDoc["gyroY"] = gyroY;
   gyroDoc["gyroZ"] = gyroZ;
   gyroDoc["acceloX"] = AcceloX;
   gyroDoc["acceloY"] = AcceloY;
   gyroDoc["acceloZ"] = AcceloZ;
   gyroDoc["Temp"] = temp;

   strAddrSecondString = flash.getAddress(gyroDoc.size());
   Serial.println(F("Generate JSON file!"));

  // Serialize JSON to file
   String gyrobuf;
   serializeJson(gyroDoc, gyrobuf);

   if (flash.writeStr(strAddrSecondString, gyrobuf)) {
    Serial.print(F("OK, wrote on address "));
    Serial.println(strAddrSecondString);


    pushToStack(strAddrSecondString);
  } else {
    Serial.print(F("Error writing to flash. Error code: "));
   
}
  }
  }
 }
