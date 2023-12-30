#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
float gyroX, gyroY, gyroZ;
int16_t rawGyroX, rawGyroY, rawGyroZ;
int16_t rawAcceloX, rawAcceloY, rawAcceloZ;
float AcceloX, AcceloY, AcceloZ;
int16_t rawTemp;
float temp;
const int gyroThreshold = 10;     // Adjust this threshold based on your experimentation for gyroscope
const int accelThreshold = 20;   // Adjust this threshold based on your experimentation for accelerometer

void setup() {
  Serial.begin(9600);
  Wire.begin();
  mpu.initialize();
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

void loop() {
  // Read gyroscope data
  gyro_data();
 
  if (AcceloY < -accelThreshold) {
    Serial.println("Harsh Braking!");
  }

  if (AcceloY > accelThreshold) {
    Serial.println("Harsh Acceleration!");
  }

  if (abs(gyroZ) > gyroThreshold) {
    Serial.println("Rash Turning!");
   }
}
