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

const int STACK_SIZE = 100; // Adjust the stack size as needed
unsigned long addressStack[STACK_SIZE];
int stackIndex = 0;

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

  // Push the address onto the stack
  pushToStack(strAddrSecondString);

  String outputString = "";
  if (flash.readStr(strAddrSecondString, outputString)) {
    Serial.print(F("Read json: "));
    Serial.println(outputString);
    Serial.print(F("From address: "));
    Serial.println(strAddrSecondString);
  }

  // Uncomment the following lines to send data to the server in LIFO manner
   sendToServer(outputString);

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

void pushToStack(unsigned long address) {
  if (stackIndex < STACK_SIZE) {
    addressStack[stackIndex++] = address;
  } else {
    // Stack is full, overwrite the oldest entry
    for (int i = 0; i < STACK_SIZE - 1; ++i) {
      addressStack[i] = addressStack[i + 1];
    }
    addressStack[STACK_SIZE - 1] = address;
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

  // Pop the address from the stack and send corresponding data to the server
  unsigned long addressToSend = popFromStack();
  if (addressToSend != 0) {
    String dataToSend;
    if (flash.readStr(addressToSend, dataToSend)) {
      sendCommand("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");
      sendCommand("AT+CIPSEND");

       String url = "GET https://api.thingspeak.com/update?api_key=KSBMDFCIB8A1IP3N&field1=" +
               String(gyroX) + "&field2=" + String(gyroY) + "&field3=" + String(gyroZ) +
               "&field4=" + String(AcceloX) + "&field5=" + String(AcceloY) + "&field6=" + String(AcceloZ) +
               "&field7=" + String(temp);

  Serial.println(url);
  gprsSerial.println(url);

      gprsSerial.println((char)26); // Sending
      delay(5000);           // Wait for reply

      sendCommand("AT+CIPSHUT"); // Close the connection
    }
  }
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
