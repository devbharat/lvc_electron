/*
 * Project test_proj_1
 * Description:
 * Author:
 * Date:
 */

#include <ArduinoJson.h>
#include "rpiDataHandler.h"
// #include "google-maps-device-locator.h"

// Globals
const size_t READ_BUF_SIZE = 512;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
size_t readBufOffset_max = 0;
size_t readBufOffset_total = 0;

rpiDataHandler handle;
int rc_switch_pin = D7;

// Battery Level
FuelGauge fuel;
double bat_soc = 0;
int bat_soc_loop_ctr = 0;

// Particle connection status ctr
int disconn_ctr = 0;

// GoogleMapsDeviceLocator locator;

void setup() {
    /* Register handle to Cloud */
    handle.initialize();
    handle.setPeriodic(2);

    pinMode(rc_switch_pin, OUTPUT);
    switch_rc("TAKEOFF");    // initialize with takeoff RC
    float bat_soc_fl = fuel.getSoC();
    bat_soc = (double)((bat_soc_fl * 100)/100);
    Particle.variable("elc_soc", bat_soc);
    Particle.function("switch_rc", switch_rc);
    // locator.withLocatePeriodic(5); 
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

void send_HB(bool conn) {
    StaticJsonBuffer<READ_BUF_SIZE> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    if (conn) {
        root["HB"] = "CONN";

    } else {
        root["HB"] = "DCONN";
    }
    root["time"] = std::string(Time.format(TIME_FORMAT_ISO8601_FULL));
    root.printTo(Serial);
}

int pin_set(int val) {
    if (val == 1) {
        digitalWrite(rc_switch_pin, HIGH);
        return 0;

    } else {
        digitalWrite(rc_switch_pin, LOW);
        return 0;
    }
}

int switch_rc(String command) {
    if (command == "TAKEOFF") {
        return pin_set(1);

    } else if (command == "LAND") {
        return pin_set(0);

    } else {
        return -1;
    }
}


void loop() {
    static bool once = false;
    if (!once && Particle.connected()) {
        Particle.keepAlive(30); // send a ping every 30 seconds
        once = true;
    }

    delay(500);

    // locator.loop();
    handle.loop();

//    if (Particle.connected()) {
//        send_HB(true);
//        disconn_ctr = 0;  // Reset disconnect counter
//
//    } else {
//        send_HB(false);
//        disconn_ctr++;
//        if (disconn_ctr > 20) {
//            System.reset();  // Call Hard Reset on disconnection
//            disconn_ctr = 0;  // For completeness
//        }
//    }

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

        // Particle.process();  // keep cellular conn
    }

    if (handle.do_RTH()) {
            send_RTH();

    } else if (handle.do_disarm()) {
            send_disarm();
    }

    // handle.reset();
    // And repeat!

//    bat_soc_loop_ctr++;
//
//    if (bat_soc_loop_ctr >= 20) {  // With 500ms loop delay, 20 iterations is 10sec update rate
//        float bat_soc_fl = fuel.getSoC();
//        bat_soc = (double)((bat_soc_fl * 100)/100);
//        bat_soc_loop_ctr = 0;
//    }
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
        if (root.containsKey("lat") && root.containsKey("lng") && root.containsKey("alt") && root.containsKey("cog")) {
            handle.parse_position(root["lat"], root["lng"], root["alt"], root["cog"]);
            // locator.update_lat(root["lat"]);
            // locator.update_lon(root["lng"]);            

        } else if (root.containsKey("RTH")) {
            handle.parse_RTH_status(root["RTH"]);

        } else if (root.containsKey("DISARM")) {
            handle.parse_DISARM_status(root["DISARM"]);

        } else if (root.containsKey("base_mode") && root.containsKey("c_mode_h") && root.containsKey("c_mode_l")) {
            handle.parse_HB_status(root["base_mode"], root["c_mode_h"], root["c_mode_l"]);

        } else if (root.containsKey("bat_v") && root.containsKey("bat_cap")) {
            handle.parse_battery_status(root["bat_v"], root["bat_cap"]);

        } else {
            Serial.printlnf("Unknown key combination %s", readBuf);
        }
    }
}
