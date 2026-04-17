#pragma once
#include "../Types/Position.h"

class Entity
{
// Variables
public:

protected:
    int id;
    Position position = {0, 0};
    
// Methods
public:
    int GetId() const
    {
        return id;
    }
    
    Position GetPosition() const
    {
        return position;
    }
    
    void SetPosition(float x, float y)
    {
        position = Position{x,y};
    }
    
    void SetPosition(Position newPosition)
    {
        position = newPosition;
    }
    
    Entity(int id) : id(id) {}
    virtual ~Entity() = default;

};
