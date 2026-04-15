#pragma once
#include "StationaryEntity.h"

class Shovel : public StationaryEntity
{
public:
    Shovel(int id) : StationaryEntity(id) {}
    
private:
    int loadSpeed;
};
