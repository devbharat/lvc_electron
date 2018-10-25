/**
 * @file rpiDataHandler.h
 * @brief Header file for implementing electron rpi data handling.
 * @author Bharat Tak
 */

#ifndef __RPI_DATA_HANDLER__H__
#define __RPI_DATA_HANDLER__H__

#include "Particle.h"

class rpiDataHandler
{
private:
    bool _initialization_complete;

    void _send_connect();

    /* Command RPi by sending serialized ArduinoJson object */
    int _command(String command);
    int _command_RTH();
    int _command_Disarm(bool reset = false);
    int _command_Disarm_for_real();

private:
    double _lat, _lng, _alt, _cog;
    int32_t _v_bat, _cap_bat;
    int32_t _base_mode;
    int32_t _custom_mode_high, _custom_mode_low;

    bool _do_rth;
    bool _do_disarm;

    bool _RTH_comm_success;
    bool _DISARM_comm_success;

    static const int CONNECT_WAIT_STATE = 0;
    static const int CONNECTED_WAIT_STATE = 2;
    static const int CONNECTED_STATE = 3;
    static const int IDLE_STATE = 4;

    unsigned long periodMs;
    String eventName;
    unsigned long stateTime;
    int state;
    unsigned long waitAfterConnect;

    const char *getDataChar();
    void publishLocation();

public:
    rpiDataHandler();
    ~rpiDataHandler();

    /* Register the cloud variables and functions */
    void initialize();

    void setPeriodic(unsigned long secondsPeriodic);
    void setEventName(const char *name);

    /* Reset Json object */
    void reset();

    bool do_RTH() {return _do_rth;};
    bool do_disarm() {return _do_disarm;};

    void loop();

    /* Update local variables on receiving serialized ArduinoJson objects */
    void parse_position(const int32_t lat, const int32_t lng, const int32_t alt, const int32_t cog);
    void parse_battery_status(const int32_t v_bat, const int32_t cap_bat);
    void parse_RTH_status(const int result);
    void parse_DISARM_status(const int result);
    void parse_HB_status(const int32_t base_mode, const int32_t custom_mode_high, const int32_t custom_mode_low);
};


#endif // __RPI_DATA_HANDLER__H__
