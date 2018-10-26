#pragma once

// #include <AP_Common/AP_Common.h>
// #include <AP_Math/AP_Math.h>

#include "polygon.h"
#include "Particle.h"

#define MAX_POINTS_GEOFENCE 10
#define GEOFENCE_LOOP_INTERVAL 1000  // 1 sec
#define GEOFENCE_PUB_INTERVAL 20000  // 20 sec

class AC_PolyFence_loader
{
private:
    const struct fence_cord_1 {
        // Add your geofence points here
        Vector2l points[MAX_POINTS_GEOFENCE] = {Vector2l(473185540,84451760),
                                            Vector2l(472983020, 84294710),
                                            Vector2l(472974280, 84480880),
                                            Vector2l(473164010, 84600130),
                                            Vector2l(473185540, 84451760),  // make sure the last point is same as first
                                            Vector2l(0, 0),
                                            Vector2l(0, 0),
                                            Vector2l(0, 0),
                                            Vector2l(0, 0),
                                            Vector2l(0, 0)};
        int num_points = 5;  // Make sure the number match non zero points above
        bool enable_fence = true;  // Set to false to disable fence
        int id = 1;  // Increment id if you change fence
    } c_1;

    Vector2l current_pos;
    bool fence_valid, position_updated;
    unsigned long checkTime, lastpubTime;
    bool geofence_breached;


public:
    AC_PolyFence_loader();
    ~AC_PolyFence_loader();

    // maximum number of fence points we can store in eeprom
    uint8_t max_points() const;

    bool init();
    bool check_fence_boundary();

    bool fence_breached();

    void loop_check();
    void setPosition(int32_t lat, int32_t lng);
    bool getGeoFenceBreached() {return geofence_breached;};

    void publishFence();
    const char *getFenceDataChar();

    // validate array of boundary points (expressed as either floats or long ints)
    //   contains_return_point should be true for plane which stores the return point as the first point in the array
    //   returns true if boundary is valid
    template <typename T>
    bool boundary_valid(uint16_t num_points, const Vector2<T>* points, bool contains_return_point) const;

    // check if a location (expressed as either floats or long ints) is within the boundary
    //   contains_return_point should be true for plane which stores the return point as the first point in the array
    //   returns true if location is outside the boundary
    template <typename T>
    bool boundary_breached(const Vector2<T>& location, uint16_t num_points, const Vector2<T>* points, bool contains_return_point) const;

};
