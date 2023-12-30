#include <SPIMemory.h>
#include <ArduinoJson.h>

SPIFlash flash(5 ,&SPI);

void setup() {
  Serial.begin(115200);

  while (!Serial)
    ; // Wait for Serial monitor to open
  delay(100);

 // flash.setClock(40000000);
  flash.begin();

}

void loop() {
  Serial.print(F("Flash size: "));
  Serial.print((long)(flash.getCapacity() / 1000));
  Serial.println(F("Kb"));

  unsigned long strAddr = 0;
  unsigned long strAddrSecondString = 0;

  // Allocate a temporary JsonDocument
  DynamicJsonDocument gyroDoc(512);

  // Set the values in the document
               gyroDoc["gyroX"] = 123.45;  
  gyroDoc["gyroY"] = 678.90; 
  gyroDoc["gyroZ"] = 45.67;   
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

  // Serial.println(F("Generate JSON file!"));

  // // Allocate a temporary JsonDocument
  // DynamicJsonDocument doc2(512);

  // // Set the values in the document
  // doc2["energyLifetime"] = 222;
  // doc2["energyYearly"] = 333;

  // Serial.println();
  // Serial.print(F("Check first free sector: "));
  // strAddrSecondString = flash.getAddress(doc2.size());
  // Serial.println(strAddrSecondString);
  // Serial.println();

  // Serial.print(F("Stream data in flash memory!"));

  // Serial.print(F("Put data in a buffer.."));

  // // Serialize JSON to file
  // String buf2;
  // serializeJson(doc2, buf2);

  // if (flash.writeStr(strAddrSecondString, buf2)) {
  //   Serial.print(F("OK, wrote on address "));
  //   Serial.println(strAddrSecondString);
  // } else {
  //   Serial.println(F("KO"));
  // }

  // String outputString2 = "";
  // if (flash.readStr(strAddrSecondString, outputString2)) {
  //   Serial.print(F("Read data: "));
  //   Serial.println(outputString2);
  //   Serial.print(F("From address: "));
  //   Serial.println(strAddrSecondString);
  // }

  while (!flash.eraseSector(strAddr))
    ;
  while (!flash.eraseSector(strAddrSecondString))
    ;

delay(10000);
  // Your main code goes here
}
