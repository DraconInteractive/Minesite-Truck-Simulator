#include "Truck.h"

int Truck::RollForFailure()
{
    int worstFail = -1;
    float worstHealth = 1.0f;

    for (int i = 0; i < parts.size(); i++)
    {
        float failChance = parts[i].baseFailRate / std::max(parts[i].health, 0.01f);
        if ((rand() / static_cast<float>(RAND_MAX)) < failChance && parts[i].health < worstHealth)
        {
            worstFail = i;
            worstHealth = parts[i].health;
        }
    }

    if (worstFail >= 0)
    {
        brokenPartIndex = worstFail;
        return worstFail;
    }

    return -1;
}
