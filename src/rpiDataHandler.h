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
    double _v_bat, _cap_bat;
    uint8_t _base_mode;
    uint32_t _custom_mode;

    bool _do_rth;
    bool _do_disarm;

    bool _RTH_comm_success;
    bool _DISARM_comm_success;

public:
    rpiDataHandler();
    ~rpiDataHandler();

    /* Register the cloud variables and functions */
    void initialize();

    /* Reset Json object */
    void reset();

    bool do_RTH() {return _do_rth;};
    bool do_disarm() {return _do_disarm;};

    /* Update local variables on receiving serialized ArduinoJson objects */
    void parse_position(const int32_t lat, const int32_t lng, const int32_t alt, const int32_t cog);
    void parse_battery_status(const double v_bat, const double cap_bat);
    void parse_RTH_status(const int result);
    void parse_DISARM_status(const int result);
    void parse_HB_status(const uint8_t base_mode, const uint32_t custom_mode);
};


#endif // __RPI_DATA_HANDLER__H__
