/*
 * Project test_proj_1
 * Description:
 * Author:
 * Date:
 */

#include <ArduinoJson.h>
#include "Particle.h"

const size_t READ_BUF_SIZE = 512;
int led1 = D0; // Instead of writing D0 over and over again, we'll write led1
// You'll need to wire an LED to this one to see it blink.

int led2 = D7; // Instead of writing D7 over and over again, we'll write led2
// This one is the little blue LED on your board. On the Photon it is next to D7, and on the Core it is next to the USB jack.

int test_var = 0;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
size_t readBufOffset_max = 0;
size_t readBufOffset_total = 0;
StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;

void setup() {
  Serial.begin(57600);

  // We are going to tell our device that D0 and D7 (which we named led1 and led2 respectively) are going to be output
  // (That means that we will be sending voltage to them, rather than monitoring voltage that comes from them)

  // It's important you do this here, inside the setup() function rather than outside it or in the loop function.

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

// Next we have the loop function, the other essential part of a microcontroller program.
// This routine gets repeated over and over, as quickly as possible and as many times as possible, after the setup function is called.
// Note: Code that blocks for too long (like more than 5 seconds), can make weird things happen (like dropping the network connection).  The built-in delay function shown below safely interleaves required background activity, so arbitrarily long delays can safely be done if you need them.

void loop() {
  // To blink the LED, first we'll turn it on...
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);

  // We'll leave it on for 1 second...
  delay(1000);

  // Then we'll turn it off...
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  // Wait 1 second...
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