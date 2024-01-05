#include <otadrive_esp.h>
#include <Arduino.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>

#include <TFT_eSPI.h>

#define APIKEY "42fcb62d-2ad9-4ac3-8cde-291979cdd417" // OTAdrive APIkey for this product
#define FW_VER "v@8.5.3"                              // this app version

// GSM Modem Serial interface
HardwareSerial mySerial(1);
// AT Command channel
TinyGsm modem(mySerial);
// MuxChannel 0 for MQTT
TinyGsmClient gsm_mqtt_client(modem, 0);
// MuxChannel 1 for OTA
TinyGsmClient gsm_otadrive_client(modem, 1);

String gprs_ip;
int gprs_signal = -1;
int counter = 0;

const int ledPin = 4; // Define the LED pin

void printInfo()
{
  Serial.printf("Example08: OTA through GSM. %s, Serial:%s, IP:%s\r\n\r\n", FW_VER, OTADRIVE.getChipId().c_str(), gprs_ip.c_str());
}

void update_prgs(size_t downloaded, size_t total)
{
  int percent = downloaded / (total / 100 + 1);
  Serial.printf("upgrade %d/%d  %d%%\r\n\r\n", downloaded, total, percent);
}

void setup()
{
  Serial.begin(115200);
  mySerial.begin(115200, SERIAL_8N1, 17, 16);

  pinMode(ledPin, OUTPUT); // Initialize the LED pin as an output
  digitalWrite(ledPin, LOW); // Turn off the LED initially

  printInfo();
  SPIFFS.begin(true);
  OTADRIVE.setInfo(APIKEY, FW_VER);
  OTADRIVE.onUpdateFirmwareProgress(update_prgs);
}

bool modem_ready = false;
void loop()
{
  for (uint8_t tr = 0; tr < 5; tr++)
  {
    printInfo();
    delay(1000);
    if (!modem.testAT(100))
    {
      Serial.println("Testing modem RX/TX test failed\r\n");
      continue;
    }

    if (modem.getSimStatus(100) != SimStatus::SIM_READY)
    {
      Serial.println("Testing modem SIMCARD failed\r\n");
      continue;
    }

    gprs_signal = modem.getSignalQuality();
    Serial.printf("Signal:%d\r\n\r\n", gprs_signal);

    if (gprs_signal > 10)
    {
      if (!modem.isGprsConnected())
      {
        Serial.println("Testing modem internet failed. Try to connect ...\r\n");
        modem.gprsConnect("airtelgprs.com");
        continue;
      }
    }
    else
    {
      continue;
    }

    gprs_ip = modem.getLocalIP();
    modem_ready = true;
    Serial.println("Modem is ready\r\n");
  }

  if (OTADRIVE.timeTick(30))
  {
    Serial.println("Let's update the firmware\r\n");
    if (modem.isGprsConnected() || modem.isGprsConnected())
    {
      auto inf = OTADRIVE.updateFirmwareInfo(gsm_otadrive_client);
      if (inf.available)
      {
        OTADRIVE.updateFirmware(gsm_otadrive_client);
      }
    }
    else
    {
      Serial.println("Modem is not ready\r\n");
    }
  }
  else
  {
    Serial.println("Waiting\r\n");
  }

  // Blink the LED every 2 seconds
  delay(2000);
  digitalWrite(ledPin, HIGH); // Turn on the LED
  delay(500);
  digitalWrite(ledPin, LOW); // Turn off the LED
}
