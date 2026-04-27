#pragma once

struct Position
{
public:
    float x = 0;
    float y = 0;

    Position operator+(const Position& other) const                 
    {                                                             
        return { x + other.x, y + other.y };                        
    }
                                                                  
    Position operator*(float scalar) const                
    {
        return { x * scalar, y * scalar };
    }

    Position operator-(const Position& other) const
    {
        return { x - other.x, y - other.y};
    }
};
