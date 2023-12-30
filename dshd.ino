#include <Arduino.h>
#include <SoftwareSerial.h>
#include <otadrive_esp.h>

//const char apn[] = "airtelgprs.com";  // Set your GSM network's APN
// const char postServer[] = "postb.in";  // Replace with your server
// const int postPort = 80;  // Adjust the port if needed
// const char postPath[] = "/your_postbin_path";  // Replace with your PostBin path

SoftwareSerial gprsSerial(17, 16);

void setup() {
  Serial.begin(115200);
  gprsSerial.begin(115200);
  delay(1000);
  pinMode(4, OUTPUT);
  // Connect to the GSM network
 gprsSerial.println("AT");
   delay(1000);

   gprsSerial.println("AT+CGATT?");
   delay(1000);

   gprsSerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
   delay(1000);
  // ShowSerialData();

   gprsSerial.println("AT+SAPBR=1,1");
   delay(1000);
  // ShowSerialData();

   gprsSerial.println("AT+SAPBR=2,1");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPINIT");
   delay(1000);
   //ShowSerialData();

   gprsSerial.println("AT+HTTPPARA=\"CID\",1");
   delay(1000);
  // Initialize OTADRIVE
  SPIFFS.begin(true);
  OTADRIVE.setInfo("f3667afb-7de7-42b0-ba23-029b08e12fa0", "1.0.0.1");
  //OTADRIVE.onUpdateFirmwareProgress(update_prgs);
 
}

void sendPostRequest() {
  String payload = "Hello, this is a test payload";

  // Set the HTTP parameters
  gprsSerial.println("AT+HTTPPARA=\"CID\",1");
   String url = "http://otadrive.com/deviceapi/update?";
  url += "k=f3667afb-7de7-42b0-ba23-029b08e12fa0";
  url += "&v=1.0.0.2";
  url += "&s=" + getChipId();
  gprsSerial.println("AT+HTTPPARA=\"URL\",\"" + url + "\"");
  //gprsSerial.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  gprsSerial.println("AT+HTTPDATA=" + String(payload.length()) + ",10000");
  gprsSerial.println(payload);

  // Send the POST request
  gprsSerial.println("AT+HTTPACTION=1");

  // Read and print the server's response
  gprsSerial.println("AT+HTTPREAD");
 

  // Terminate the HTTP session
  gprsSerial.println("AT+HTTPTERM");
  
  if (!OTADRIVE.timeTick(60))
    return;
  // Check for OTADRIVE updates
  OTADRIVE.updateFirmware();
}


String getChipId()
{
  String ChipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
  ChipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
  return ChipIdHex;
}


void loop() {

  digitalWrite(4, HIGH);
  delay(400);
  digitalWrite(4, LOW);
  delay(400);
  // Your main loop code here

  // Send a POST request and check for updates
  sendPostRequest();
  Serial.println(getChipId());

  delay(5000);  // Delay for 5 seconds before the next request
}

String sendATCommand(String command) {
  gprsSerial.println(command);
  delay(1000);
  String response = "";
  while (gprsSerial.available()) {
    char c = gprsSerial.read();
    response += c;
  }
  return response;
}
