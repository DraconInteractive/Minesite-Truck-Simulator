#pragma once
#include <string>

struct EntityPart
{
    std::string name;
    float health       = 1.0f;  // 0–1
    float repairTime   = 0.0f;  // minutes to repair this part
    float baseFailRate = 0.005f; // probability of failure per cycle at full health
    float wearPerCycle = 0.01f; // health lost per successful cycle
};
