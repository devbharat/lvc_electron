/*
 * Project test_proj_1
 * Description:
 * Author:
 * Date:
 */

#include <ArduinoJson.h>
#include "rpiDataHandler.h"

// Globals
const size_t READ_BUF_SIZE = 512;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
size_t readBufOffset_max = 0;
size_t readBufOffset_total = 0;

rpiDataHandler handle;

void setup() {
  /* Register handle to Cloud */
  handle.initialize();
  Serial.begin(57600);
}

void send_RTH() {
  StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["command"] = "RTH";
  root.printTo(Serial);
}

void send_disarm() {
  StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["command"] = "DISARM";
  root.printTo(Serial);
}

void loop() {
  delay(1000);

  readBufOffset = 0;
  while(Serial.available()) {
    if (readBufOffset < READ_BUF_SIZE) {
      char c = Serial.read();
      if ((c != '\0') ) {
        // Add character to buffer
        readBuf[readBufOffset++] = c;

      } else {
        // End of line character found, process line
        readBuf[readBufOffset] = 0;
        processBuffer();
        readBufOffset = 0;
      }

    } else {
      Serial.println("readBuf overflow, emptying buffer");
      readBufOffset = 0;
    }
  }

  if (handle.do_RTH()) {
      send_RTH();

  } else if (handle.do_disarm()) {
      send_disarm();
  }

  // handle.reset();
  // And repeat!
}

void processBuffer() {
  StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;
  // Serial.printlnf("Received from RPi: %s", readBuf);
  JsonObject& root = jsonBuffer.parseObject(readBuf);
  if (!root.success()) {
    Serial.printlnf("parseObject() failed %s", readBuf);
    return;

  } else {
    // Serial.println("worked elec");
    if (root.containsKey("lat") && root.containsKey("lng") && root.containsKey("alt")) {
      handle.parse_position(root["lat"], root["lng"], root["alt"]);

    } else if (root.containsKey("armed_state") && root.containsKey("main_state") && root.containsKey("nav_state")) {
      handle.parse_vehicle_status(root["armed_state"], root["main_state"], root["nav_state"]);

    } else if (root.containsKey("v_bat") && root.containsKey("cap_bat")) {
      handle.parse_battery_status(root["v_bat"], root["cap_bat"]);

    } else {
      Serial.printlnf("Unknown key combination %s", readBuf);
    }
  }
}