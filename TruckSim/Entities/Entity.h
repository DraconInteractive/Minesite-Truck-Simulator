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
    int GetId();
    Position GetPosition();
    
    void SetPosition(int x, int y);
    void SetPosition(Position newPosition);
    
    Entity(int id) : id(id) {}
    virtual ~Entity() = default;

};
