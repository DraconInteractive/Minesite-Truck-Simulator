#pragma once
#include <cmath>

#include "../Entities/Entity.h"

namespace Utilities
{
    inline double GetTravelTime(Position a, Position b, float speed)
    {
        int dx = b.x - a.x;
        int dy = b.y - a.y;
        double distance = std::sqrt(dx * dx + dy * dy);
        return distance / speed;
    }
};
