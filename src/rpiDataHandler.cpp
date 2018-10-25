/**
 * @file rpiDataHandler.cpp
 * @brief Implements communication between particle electron and rpi
 * @author Bharat Tak
 */

#include "Particle.h"
#include "rpiDataHandler.h"

static char rBuf[256];

rpiDataHandler::rpiDataHandler() : periodMs(10000), eventName("DL"),
    stateTime(0), state(CONNECT_WAIT_STATE), waitAfterConnect(8000)
{
    _initialization_complete = false;
    _lat = 0;  // Mwanza
    _lng = 0;  // Mwanza
    _alt = 0;
    _cog = 45;

    _v_bat = 0;
    _cap_bat = 0;

    _base_mode = 0;
    _custom_mode_high = 0;
    _custom_mode_low = 0;

    _do_rth = false;
    _do_disarm = false;

    _RTH_comm_success = false;
    _DISARM_comm_success = false;
}

rpiDataHandler::~rpiDataHandler()
{
}

void rpiDataHandler::_send_connect()
{

}

void rpiDataHandler::initialize()
{
    Particle.variable("lat", rpiDataHandler::_lat);
    Particle.variable("lng", rpiDataHandler::_lng);
    Particle.variable("alt", rpiDataHandler::_alt);
    Particle.variable("cog", rpiDataHandler::_cog);

    Particle.variable("v_bat", rpiDataHandler::_v_bat);
    Particle.variable("cap_bat", rpiDataHandler::_cap_bat);

    Particle.variable("base_mode", rpiDataHandler::_base_mode);
    Particle.variable("c_mode_h", rpiDataHandler::_custom_mode_high);
    Particle.variable("c_mode_l", rpiDataHandler::_custom_mode_low);

    Particle.variable("RTH_OK", rpiDataHandler::_RTH_comm_success);
    Particle.variable("DISARM_OK", rpiDataHandler::_DISARM_comm_success);

    Particle.function("command", &rpiDataHandler::_command, this);

    _initialization_complete = true;
}

int rpiDataHandler::_command(String command)
{
    if (command == "RTH") {
        return _command_RTH();

    } else if (command == "DISARM") {
        return _command_Disarm();

    } else if (command == "RS_DISARM") {
        return _command_Disarm(true);

    } else {
        return -1;
    }
}

int rpiDataHandler::_command_RTH()
{
    _do_rth = true;
    return 0;
}

int rpiDataHandler::_command_Disarm(bool reset)
{
    static int call_count = 0;

    if (reset) {
        call_count = 0;

    } else {
        call_count++;

        if (call_count >= 3) {  // Atleast need 3 calls to actually call disarm
            call_count = 0;
            return _command_Disarm_for_real();
        }
    }

    return call_count;
}

int rpiDataHandler::_command_Disarm_for_real()
{
    _do_disarm = true;
    return 0;
}

void rpiDataHandler::reset()
{
    _do_rth = false;
    _do_disarm = false;
}


void rpiDataHandler::parse_position(int32_t lat, int32_t lng, int32_t alt, int32_t cog)
{
    _lat = lat * 0.0000001;
    _lng = lng * 0.0000001;
    _alt = alt * 0.001;
    _cog = cog * 0.01;
}

void rpiDataHandler::parse_HB_status(const int32_t base_mode, const int32_t custom_mode_high, const int32_t custom_mode_low)
{
    _base_mode = base_mode;
    _custom_mode_high = custom_mode_high;
    _custom_mode_low = custom_mode_low;
}

void rpiDataHandler::parse_battery_status(int32_t v_bat, int32_t cap_bat)
{
    _v_bat = v_bat;
    _cap_bat = cap_bat;
}

void rpiDataHandler::parse_RTH_status(const int result)
{
    if (result) {
        _RTH_comm_success = true;

    } else {
        _RTH_comm_success = false;
    }

    _do_rth = false;
}

void rpiDataHandler::parse_DISARM_status(const int result)
{
    if (result) {
        _DISARM_comm_success = true;

    } else {
        _DISARM_comm_success = false;
    }

    _do_disarm = false;
    _command_Disarm(true);
}

void rpiDataHandler::loop()
{
    switch(state) {
    case CONNECT_WAIT_STATE:
        if (Particle.connected()) {
            state = CONNECTED_WAIT_STATE;
            stateTime = millis();
        }
        break;

    case CONNECTED_WAIT_STATE:
        if (millis() - stateTime >= waitAfterConnect) {
                state = CONNECTED_STATE;
                stateTime = millis() - periodMs;
        }
        break;

    case CONNECTED_STATE:
        if (Particle.connected()) {
            if (millis() - stateTime >= periodMs) {
                stateTime = millis();
                publishLocation();
            }
        }
        else {
            // We have disconnected, rec
            state = CONNECT_WAIT_STATE;
        }
        break;


    case IDLE_STATE:
        // Just hang out here forever (entered only on LOCATOR_MODE_ONCE)
        break;
    }

}

void rpiDataHandler::setPeriodic(unsigned long secondsPeriodic)
{
    if (secondsPeriodic < 2) {
        secondsPeriodic = 2;
    }
    periodMs = secondsPeriodic * 1000;
}

void rpiDataHandler::setEventName(const char *name)
{
    this->eventName = name;
}

void rpiDataHandler::publishLocation()
{
    const char *outData = getDataChar();
    if (Particle.connected()) {
        Particle.publish(eventName, outData, PRIVATE);
    }
}

const char *rpiDataHandler::getDataChar()
{
    snprintf(rBuf, sizeof(rBuf), "%s,%s,%s,%s", String(_lat).c_str(), String(_lng).c_str(), String(_alt).c_str(), String(_cog).c_str());
    return rBuf;
}