TaskHandle_t Task1;
// TaskHandle_t Task2;
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MPU6050.h>
SoftwareSerial gprsSerial(25,5);
MPU6050 mpu;
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX,rawAcceloY,rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;
File dataFile;
File myFile;
float s,lon, alt,lat,speed;
const int CS = 13;



//----------------------------------------------------

void readRawGyroData() {
  mpu.getRotation(&rawGyroX, &rawGyroY, &rawGyroZ);
}
void readRawAccelData() {
  mpu.getAcceleration(&rawAcceloX, &rawAcceloY, &rawAcceloZ);
}

void readRawTemperature() {
  rawTemp = mpu.getTemperature();
}
void convertRawToActual() {

  AcceloX=rawAcceloX/16384;
  AcceloY=rawAcceloY/16384;
  AcceloZ=rawAcceloZ/16384;

  gyroX=rawGyroX/131;
  gyroY=rawGyroY/131;
  gyroZ=rawGyroZ/131;
  
  temp=((rawTemp)/340 + 36.53);
}
// -------------------------------------------------------------------------------

// void WriteFile(const char * path, const char * message){
//   // open the file. note that only one file can be open at a time,
//   // so you have to close this one before opening another.
//   myFile = SD.open(path, FILE_WRITE);
//   // if the file opened okay, write to it:
//   if (myFile) {
//     Serial.printf("Writing to %s ", path);
//     myFile.println(message);
//     myFile.close(); // close the file:
//     Serial.println("completed.");
//   } 
//   // if the file didn't open, print an error:
//   else {
//     Serial.println("error opening file ");
//     Serial.println(path);
//   }
// }

//-------------------------------------------------
// void ReadFile(const char * path){
//   // open the file for reading:
//   myFile = SD.open(path);
//   if (myFile) {
//      Serial.printf("Reading file from %s\n", path);
//      // read from the file until there's nothing else in it:
//     while (myFile.available()) {
//       Serial.write(myFile.read());
//     }
//     myFile.close(); // close the file:
//   } 
//   else {
//     // if the file didn't open, print an error:
//     Serial.println("error opening gyro.txt");
//   }
// }

//-------------------------------------------------------
void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
  
}

//-------------------------------------------------------
//-------------------------------------------------------
void setup()
{
  Serial.begin(9600);    // the GPRS baud rate  
  gprsSerial.begin(9600);  // connect serial
  Wire.begin();
  mpu.initialize();              // the GPRS baud rate   
      // the GPRS baud rate  
  

xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
// xTaskCreatePinnedToCore(
//                     Task2code,   /* Task function. */
//                     "Task2",     /* name of task. */
//                     10000,       /* Stack size of task */
//                     NULL,        /* parameter of the task */
//                     1,           /* priority of the task */
//                     &Task2,      /* Task handle to keep track of created task */
//                     0);          /* pin task to core 1 */
  
}
 

// void Task1code( void * pvParameters ){
//   Serial.print("Task1 running on core ");
//   Serial.println(xPortGetCoreID());


//   for(;;){

//   // if (!SD.begin(CS)) {
//   //   Serial.println("initialization failed!");
//   //   return;
//   // }
//   // Serial.println("initialization done.");
//   // //gpsdata();
//   // dataFile = SD.open("gyro.txt", FILE_WRITE);
//   // dataFile.write(AcceloX);
//   // dataFile.write(AcceloY);
//   // dataFile.write(AcceloZ);
//   // dataFile.write(gyroX);
//   // dataFile.write(gyroY);
//   // dataFile.write(gyroZ);
//   // dataFile.write(temp);
//   // dataFile.println(AcceloX);
//   // dataFile.println(AcceloY);
//   // dataFile.println(AcceloZ);
//   // dataFile.println(gyroX);
//   // dataFile.println(gyroY);
//   // dataFile.println(gyroZ);
//   // dataFile.println(temp);
  
  

//   // dataFile.read();
//   // //SD.close("gyro.txt");

//   // } 
//    readRawGyroData();
//   readRawTemperature();
//   convertRawToActual();

//   if (gprsSerial.available())
//     Serial.write(gprsSerial.read());
 
//   gprsSerial.println("AT");
//   delay(1000);
 
//   gprsSerial.println("AT+CPIN?");
//   delay(1000);
 
//   gprsSerial.println("AT+CREG?");
//   delay(1000);
 
//   gprsSerial.println("AT+CGATT?");
//   delay(1000);
 
//   gprsSerial.println("AT+CIPSHUT");
//   delay(1000);
 
//   gprsSerial.println("AT+CIPSTATUS");
//   delay(2000);
 
//   gprsSerial.println("AT+CIPMUX=0");
//   delay(2000);
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
//   delay(1000);
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CIICR");//bring up wireless connection
//   delay(3000);
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CIFSR");//get local IP adress
//   delay(2000);
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CIPSPRT=0");
//   delay(3000);
 
//   ShowSerialData();
  
//   gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
//   delay(6000);
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CIPSEND");//begin send data to remote server
//   delay(4000);
//   ShowSerialData();
  
//   String str="GET https://api.thingspeak.com/update?api_key=HO11CFE808IQI7FB&field1=" + String(AcceloX) +"&field2=" + String(AcceloY) +"&field3=" + String(AcceloZ) +"&field4=" + String(gyroX) +"&field5=" + String(gyroY) +"&field6=" + String(gyroZ) +"&field7=" + String(temp);
//   Serial.println(str);
//   gprsSerial.println(str);//begin send data to remote server
  
//   delay(4000);
//   ShowSerialData();
 
//   gprsSerial.println((char)26);//sending
//   delay(5000);//waitting for reply, important! the time is base on the condition of internet 
//   gprsSerial.println();
 
//   ShowSerialData();
 
//   gprsSerial.println("AT+CIPSHUT");//close the connection
//   delay(1000);
//   ShowSerialData();
// }

// }


void Task1code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());


  for(;;){

// readRawAccelData();
  readRawGyroData();
  readRawTemperature();
  convertRawToActual();


  Serial.print("Accelo X: ");
  Serial.print(AcceloX);
  Serial.print(" g, Accelo Y: ");
  Serial.print(AcceloY);
  Serial.print(" g, Accelo Z: ");
  Serial.print(AcceloZ);
  Serial.println(" g");


  Serial.print("Gyro X: ");
  Serial.print(gyroX);
  Serial.print(" deg/s, Gyro Y: ");
  Serial.print(gyroY);
  Serial.print(" deg/s, Gyro Z: ");
  Serial.print(gyroZ);
  Serial.println(" deg/s");

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" oC");

  }}
      
   
  // if (gprsSerial.available())
  //   Serial.write(gprsSerial.read());
 
  // gprsSerial.println("AT");
  // delay(1000);
 
  // gprsSerial.println("AT+CPIN?");
  // delay(1000);
 
  // gprsSerial.println("AT+CREG?");
  // delay(1000);
 
  // gprsSerial.println("AT+CGATT?");
  // delay(1000);
 
  // gprsSerial.println("AT+CIPSHUT");
  // delay(1000);
 
  // gprsSerial.println("AT+CIPSTATUS");
  // delay(2000);
 
  // gprsSerial.println("AT+CIPMUX=0");
  // delay(2000);
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  // delay(1000);
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CIICR");//bring up wireless connection
  // delay(3000);
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CIFSR");//get local IP adress
  // delay(2000);
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CIPSPRT=0");
  // delay(3000);
 
  // ShowSerialData();
  
  // gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  // delay(6000);
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  // delay(4000);
  // ShowSerialData();
  
  // String str="GET https://api.thingspeak.com/update?api_key=HO11CFE808IQI7FB&field1=" + String(AcceloX) +"&field2=" + String(AcceloY) +"&field3=" + String(AcceloZ) +"&field4=" + String(gyroX) +"&field5=" + String(gyroY) +"&field6=" + String(gyroZ) +"&field7=" + String(temp);
  // Serial.println(str);
  // gprsSerial.println(str);//begin send data to remote server
  
  // delay(4000);
  // ShowSerialData();
 
  // gprsSerial.println((char)26);//sending
  // delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  // gprsSerial.println();
 
  // ShowSerialData();
 
  // gprsSerial.println("AT+CIPSHUT");//close the connection
  // delay(1000);
  // ShowSerialData();
// }
// }


void loop(){
  readRawGyroData();
  readRawTemperature();
  convertRawToActual();
  if (gprsSerial.available())
    Serial.write(gprsSerial.read());
 
  gprsSerial.println("AT");
  delay(1000);
 
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
 
  ShowSerialData();
 
  gprsSerial.println("AT+CSTT=\"airtelgprs.com\"");//start task and setting the APN,
  delay(1000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSPRT=0");
  delay(3000);
 
  ShowSerialData();
  
  gprsSerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  ShowSerialData();
  
  String str="GET https://api.thingspeak.com/update?api_key=HO11CFE808IQI7FB&field1=" + String(AcceloX) +"&field2=" + String(AcceloY) +"&field3=" + String(AcceloZ) +"&field4=" + String(gyroX) +"&field5=" + String(gyroY) +"&field6=" + String(gyroZ) +"&field7=" + String(temp);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(1000);
  ShowSerialData();
}
