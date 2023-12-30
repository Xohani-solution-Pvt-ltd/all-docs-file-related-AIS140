#include <TinyGPS++.h>  // library for gps
#include <SoftwareSerial.h>   // library for take connection between serial monitor and microcontroller

#include <Wire.h>  // library for gyro data 
#include <MPU6050.h> // library for gyr data 

// make object of gyro library
MPU6050 mpu;

// define value of rx nad tx of simmodule 
SoftwareSerial gprsSerial(6,25);

// define value of GPS and pin number of  rx and tx of GPS 
SoftwareSerial gpsSerial(5,25);
TinyGPSPlus gps;
float s,lat,lon,alt,speed;
      
int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX,rawAcceloY,rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;

void setup()
{
  Serial.begin(115200); // connect serial
  gprsSerial.begin(115200); // connect gprs
  gpsSerial.begin(115200); // connect gps
  Wire.begin();
  mpu.initialize(); 
}
// fetch gyro data
void fetch_gyrodata(){

  mpu.getRotation(&rawGyroX, &rawGyroY, &rawGyroZ);
  mpu.getAcceleration(&rawAcceloX, &rawAcceloY, &rawAcceloZ);
  rawTemp = mpu.getTemperature();

  AcceloX=rawAcceloX/16384;
  AcceloY=rawAcceloY/16384;
  AcceloZ=rawAcceloZ/16384;

  gyroX=rawGyroX/131;
  gyroY=rawGyroY/131;
  gyroZ=rawGyroZ/131;
  
  temp=((rawTemp)/340 + 36.53);


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

  delay(3000);

}
void internet_connectivity(){

if (gprsSerial.available())
    Serial.write(gprsSerial.read());
 
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
  
}

// show data on serial monitor
void ShowSerialData()
{
  while(gprsSerial.available()!=0)
  Serial.write(gprsSerial.read());
  delay(5000); 
}

void loop()
{
  // *check for gps related data
  if (millis() > 50000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
       s=gps.satellites.value();
       lat=gps.location.lat();
       lon=gps.location.lng();
       alt=gps.altitude.meters();
       speed=gps.speed.mps();

      Serial.print("SATS: ");
      Serial.println(s);
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("ALT: ");
      Serial.println(alt);
      Serial.print("SPEED: ");
      Serial.println(speed);
    }
  }

//  print gyrodata 
   fetch_gyrodata();

  
//  establish connection betweenw gprs and server   
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
  
  String str="GET https://api.thingspeak.com/update?api_key=SKYVIFIGFJM6W4QA&field1=" + String(s) +"&field2=" + String(lat) +"&field3=" + String(lon) +"&field4=" + String(alt) +"&field5=" + String(speed);
  Serial.println(str);
  gprsSerial.println(str);//begin send data to remote server
  
  delay(4000);
  ShowSerialData();
 
  gprsSerial.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  gprsSerial.println();
 
  ShowSerialData();
 
  gprsSerial.println("AT+CIPSHUT");//close the connection
  delay(100);
  ShowSerialData();
}