#pragma once
#include "StationaryEntity.h"

class Shovel : public StationaryEntity
{
public:
    Shovel(int id_, int loadSpeed) : StationaryEntity(id_), loadSpeed(loadSpeed) {}
    
private:
    int loadSpeed = 0;
};
