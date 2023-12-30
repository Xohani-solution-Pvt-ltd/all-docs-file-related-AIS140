#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  FSInfo fs_info;
  SPIFFS.info(fs_info);

  Serial.println("File system information:");
  Serial.printf("Total bytes: %u\n", fs_info.totalBytes);
  Serial.printf("Used bytes: %u\n", fs_info.usedBytes);
  Serial.printf("Free bytes: %u\n", fs_info.totalBytes - fs_info.usedBytes);

  float usedPercentage = (float)fs_info.usedBytes / fs_info.totalBytes * 100;
  Serial.printf("Used percentage: %.2f%%\n", usedPercentage);
}

void loop() {
  // Your main code here
}
