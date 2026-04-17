#pragma once

struct Position
{
public:
    float x;
    float y;

    Position operator+(const Position& other) const                 
    {                                                             
        return { x + other.x, y + other.y };                        
    }
                                                                  
    Position operator*(float scalar) const                
    {
        return { x * scalar, y * scalar };
    }

};
