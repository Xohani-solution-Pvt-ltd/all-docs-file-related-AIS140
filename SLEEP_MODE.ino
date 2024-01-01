#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

int BLUE_LED_PIN = 4;
int GREEN_LED_PIN = 1;

void setup(){
  Serial.begin(9600);
  pinMode(BLUE_LED_PIN,OUTPUT);
  pinMode(GREEN_LED_PIN,OUTPUT);
  delay(500);
  
  if(bootCount == 0) //Run this only the first time
  {
      digitalWrite(GREEN_LED_PIN,HIGH);
      bootCount = bootCount+1;
  }else
  {
      digitalWrite(BLUE_LED_PIN,HIGH);
  }
  
  delay(3000);

  digitalWrite(BLUE_LED_PIN,LOW);
  digitalWrite(GREEN_LED_PIN,LOW);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop(){
  Serial.print("Sleep count :");
  Serial.println(bootCount);
}
