const int analogInPin = 34;  // Analog input pin

void setup() {
  Serial.begin(9600);  // Initialize serial communication
 
}

void loop() {
  printVolts();
}

void printVolts() {
  int sensorValue = analogRead(analogInPin);  // Read the analog input
  float voltage = sensorValue * (5.0 / 1023.0);  // Convert the value to true voltage for a 12V battery
  Serial.println(sensorValue);
  int batteryPercentage = map(sensorValue, 0, 1023, 0, 100);  // Map the sensor value to a percentage (assuming full scale is 1023)

  Serial.print("Voltage: ");
  Serial.print(voltage, 2);  // Print voltage with 2 decimal places
  Serial.print(" V, Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println("%");

 
}
