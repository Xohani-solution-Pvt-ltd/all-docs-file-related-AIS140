#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

int16_t rawGyroX, rawGyroY, rawGyroZ;
float gyroX, gyroY, gyroZ;
int16_t rawAcceloX,rawAcceloY,rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();
}

void loop() {
  // readRawAccelData();
  // readRawGyroData();
  // readRawTemperature();
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

  delay(3000);
}

void convertRawToActual() {
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
}