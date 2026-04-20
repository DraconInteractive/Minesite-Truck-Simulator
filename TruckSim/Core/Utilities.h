#pragma once
#include <cmath>

#include "../Entities/Entity.h"

namespace Utilities
{
    inline float GetTravelTime(Position a, Position b, float speed)
    {
        // Position is in km, speed is in km per hour
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        return (distance / speed) * 60; // return time in km/minute to fit in time step
    }
};