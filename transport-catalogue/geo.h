#pragma once

#include <cmath>

namespace geo {

    const int EARTH_RADIUS = 6371000;

    struct Coordinates {
        double lat = 0.0;
        double lng = 0.0;
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);

} // namespace geo