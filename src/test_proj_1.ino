/*
 * Project test_proj_1
 * Description:
 * Author:
 * Date:
 */

#include <ArduinoJson.h>
#include "Particle.h"

const size_t READ_BUF_SIZE = 512;
int led1 = D0;
int led2 = D7;
int test_var = 0;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
size_t readBufOffset_max = 0;
size_t readBufOffset_total = 0;
StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;

void setup() {
  Serial.begin(57600);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  Particle.variable("test_var", test_var);
  char json[] =
      "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  const char* sensor = root["sensor"];
  long time = root["time"];
  double latitude = root["data"][0];
  double longitude = root["data"][1];

  // Print values.
  Serial.println(sensor);
  Serial.println(time);
  Serial.println(latitude, 6);
  Serial.println(longitude, 6);
}


void loop() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);

  delay(1000);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  delay(1000);

  test_var = test_var + 1;
  Serial.println(test_var);
  Serial.println(readBufOffset_max);
  Serial.println(readBufOffset_total);

  readBufOffset = 0;
  while(Serial.available()) {
    if (readBufOffset < READ_BUF_SIZE) {
      char c = Serial.read();
      if ((c != '\0') ) {
        // Add character to buffer
        readBuf[readBufOffset++] = c;
        readBufOffset_total++;
      }
      else {
        // End of line character found, process line
        readBuf[readBufOffset] = 0;
        readBufOffset_max = readBufOffset;
        processBuffer();
        readBufOffset = 0;
      }
    }
    else {
      Serial.println("readBuf overflow, emptying buffer");
      readBufOffset = 0;
    }
  }

  // And repeat!
}

void processBuffer() {
  StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer1;
  Serial.printlnf("Received from RPi: %s", readBuf);
  JsonObject& root1 = jsonBuffer1.parseObject(readBuf);
  if (!root1.success()) {
    Serial.printlnf("parseObject() failed %s", readBuf);
    return;
  } else {
    Serial.println("worked elec");
  }
}