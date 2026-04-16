#pragma once
#include "../Entity.h"

class StationaryEntity : public Entity
{
public:
    StationaryEntity(int id_, Position _position) : Entity(id_)
    {
        SetPosition(_position);
    }
};
