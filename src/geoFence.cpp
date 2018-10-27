#include "geoFence.h"

static char fBuf[512];


AC_PolyFence_loader::AC_PolyFence_loader() : current_pos(0, 0), checkTime(0), lastpubTime(0)
{
    fence_valid = false;
    position_updated = false;
    geofence_breached = false;
    checkTime = millis();
    lastpubTime = millis();
}

AC_PolyFence_loader::~AC_PolyFence_loader()
{
}

/*
  maximum number of fencepoints
 */
uint8_t AC_PolyFence_loader::max_points() const
{
    return MAX_POINTS_GEOFENCE;
}

bool AC_PolyFence_loader::init()
{
    if (check_fence_boundary()) {
        fence_valid = true;
        return true;
    }

    return false;
}

bool AC_PolyFence_loader::check_fence_boundary()
{
    return c_1.enable_fence && boundary_valid(c_1.num_points, c_1.points, false);
}

bool AC_PolyFence_loader::fence_breached()
{
    if (fence_valid) {
        return boundary_breached(current_pos, c_1.num_points, c_1.points, false);

    } else {
        return false; // Never breach invalid fence
    }
}

void AC_PolyFence_loader::loop_check()
{
    if (millis() - checkTime >= GEOFENCE_LOOP_INTERVAL) {  // 1 sec
        checkTime = millis();
        if (position_updated && fence_valid) {
            geofence_breached = fence_breached();
        }
    }

    if (millis() - lastpubTime >= GEOFENCE_PUB_INTERVAL) {  // 10 sec
        lastpubTime = millis();
        if (fence_valid) {
            publishFence();
        }
    }

}

void AC_PolyFence_loader::setPosition(int32_t lat, int32_t lng)
{
    current_pos.x = lat;
    current_pos.y = lng;

    if (!position_updated) {
        position_updated = true;
    }
}

void AC_PolyFence_loader::publishFence()
{
    const char *outData = getFenceDataChar();
    if (Particle.connected()) {
        Serial.printlnf("%s\n", outData);
        Particle.publish("Fence", outData, PRIVATE);
    }
}

const char *AC_PolyFence_loader::getFenceDataChar()
{
    String tmp = String(c_1.id) + ";";
    tmp = tmp + String(c_1.num_points) + "_";
    for (int i = 0; i < c_1.num_points; i++) {
        float f_lat = c_1.points[i].x * 0.0000001;
        float f_lon = c_1.points[i].y * 0.0000001;
        tmp = tmp + String(f_lat) + "," + String(f_lon);
        if (i != (c_1.num_points -1)) {
            tmp = tmp + ":";
        }
    }

    snprintf(fBuf, sizeof(fBuf), "%s", tmp.c_str());
    return fBuf;
}

// validate array of boundary points (expressed as either floats or long ints)
//   contains_return_point should be true for plane which stores the return point as the first point in the array
//   returns true if boundary is valid
template <typename T>
bool AC_PolyFence_loader::boundary_valid(uint16_t num_points, const Vector2<T>* points, bool contains_return_point) const
{
    // exit immediate if no points
    if (points == nullptr) {
        return false;
    }

    // start from 2nd point if boundary contains return point (as first point)
    uint8_t start_num = contains_return_point ? 1 : 0;

    // a boundary requires at least 4 point (a triangle and last point equals first)
    if (num_points < start_num + 4) {
        return false;
    }

    // point 1 and last point must be the same.  Note: 0th point is reserved as the return point
    if (!Polygon_complete(&points[start_num], num_points-start_num)) {
        return false;
    }

    // check return point is within the fence
    if (contains_return_point && Polygon_outside(points[0], &points[1], num_points-start_num)) {
        return false;
    }

    return true;
}

// check if a location (expressed as either floats or long ints) is within the boundary
//   contains_return_point should be true for plane which stores the return point as the first point in the array
//   returns true if location is outside the boundary
template <typename T>
bool AC_PolyFence_loader::boundary_breached(const Vector2<T>& location, uint16_t num_points, const Vector2<T>* points,
                                            bool contains_return_point) const
{
    // exit immediate if no points
    if (points == nullptr) {
        return false;
    }

    // start from 2nd point if boundary contains return point (as first point)
    uint8_t start_num = contains_return_point ? 1 : 0;

    // check location is within the fence
    return Polygon_outside(location, &points[start_num], num_points-start_num);
}

// declare type specific methods
template bool AC_PolyFence_loader::boundary_valid<int32_t>(uint16_t num_points, const Vector2l* points, bool contains_return_point) const;
template bool AC_PolyFence_loader::boundary_valid<float>(uint16_t num_points, const Vector2f* points, bool contains_return_point) const;
template bool AC_PolyFence_loader::boundary_breached<int32_t>(const Vector2l& location, uint16_t num_points,
                                                              const Vector2l* points, bool contains_return_point) const;
template bool AC_PolyFence_loader::boundary_breached<float>(const Vector2f& location, uint16_t num_points,
                                                            const Vector2f* points, bool contains_return_point) const;
