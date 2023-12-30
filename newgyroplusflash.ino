#include <SoftwareSerial.h>
#include <SPIMemory.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <MPU6050.h>

SPIFlash flash(5, &SPI);
MPU6050 mpu;

SoftwareSerial gprsSerial(17, 16);

int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;

void setup() {
  Serial.begin(115200);
  flash.begin();
  Wire.begin();
  mpu.initialize();
  gprsSerial.begin(9600); // Adjust the baud rate based on your GPRS module
}

void loop() {
  readRawAccelData();
  readRawGyroData();
  readRawTemperature();
  convertRawToActual();

  Serial.print(F("Flash size: "));
  Serial.print((long)(flash.getCapacity() / 1000));
  Serial.println(F("Kb"));
  unsigned long strAddrSecondString = 0;
  
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
  } else {
    Serial.println(F("KO"));
  }

  String outputString = "";
  if (flash.readStr(strAddrSecondString, outputString)) {
    Serial.print(F("Read json: "));
    Serial.println(outputString);
    Serial.print(F("From address: "));
    Serial.println(strAddrSecondString);
  }

  

  sendToServer(gyrobuf);

  while (!flash.eraseSector(strAddrSecondString))
    ;

  delay(10000);
}

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
  AcceloX = rawAcceloX / 16384.0;
  AcceloY = rawAcceloY / 16384.0;
  AcceloZ = rawAcceloZ / 16384.0;

  gyroX = rawGyroX / 131.0;
  gyroY = rawGyroY / 131.0;
  gyroZ = rawGyroZ / 131.0;

  temp = ((rawTemp) / 340.0) + 36.53;
}

void sendToServer(String data) {
  while (gprsSerial.available())
    Serial.write(gprsSerial.read());

  sendCommand("AT");
  sendCommand("AT+CPIN?");
  sendCommand("AT+CREG?");
  sendCommand("AT+CGATT?");
  sendCommand("AT+CIPSHUT");
  sendCommand("AT+CIPSTATUS");
  sendCommand("AT+CIPMUX=0");
  sendCommand("AT+CSTT=\"airtelgprs.com\"");
  sendCommand("AT+CIICR");
  sendCommand("AT+CIFSR");
  sendCommand("AT+CIPSPRT=0");
  sendCommand("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");
  sendCommand("AT+CIPSEND");

  String url = "GET https://api.thingspeak.com/update?api_key=KSBMDFCIB8A1IP3N&field1=" +
               String(gyroX) + "&field2=" + String(gyroY) + "&field3=" + String(gyroZ) +
               "&field4=" + String(AcceloX) + "&field5=" + String(AcceloY) + "&field6=" + String(AcceloZ) +
               "&field7=" + String(temp);

 Serial.println(url);
 gprsSerial.println(url);
 gprsSerial.println((char)26);
 
  delay(5000);            // wait for reply

  sendCommand("AT+CIPSHUT"); // close the connection
}

void sendCommand(String command) {
  gprsSerial.println(command);
  delay(1000);
  Serial.print(command + " Response: ");
  while (gprsSerial.available()) {
    Serial.write(gprsSerial.read());
  }
  Serial.println();
}
