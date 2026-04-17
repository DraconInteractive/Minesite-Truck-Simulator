#pragma once
#include <iostream>

#include "MobileEntity.h"

enum class TruckState
{
    Idle,
    Travelling,
    Loading,
    Queueing,
    Dumping
};

inline std::string TruckStateToString(TruckState state)
{                                                            
    switch (state)                                           
    {                                                        
        case TruckState::Idle:       return "Idle";          
        case TruckState::Travelling: return "Travelling";
        case TruckState::Loading:    return "Loading";
        case TruckState::Queueing:   return "Queueing";
        case TruckState::Dumping:    return "Dumping";
    }
}


struct TruckId
{
    int value = -1; // Default to invalid ID
};

class Truck : public MobileEntity
{
public:
    Truck(int id_, float speed, int capacity, int currentLoad) : MobileEntity(id_, speed), capacity(capacity), currentLoad(currentLoad) {} // TODO make a sort of construction DTO so we can create an entity from a config

    TruckState GetState() const
    {
        return state;
    }
    
    void SetState(TruckState newState)
    {
        state = newState;
        std::cout << "Truck " << id << " is now " + TruckStateToString(state) << "\n";
    }

    // Just for rendering right now, so we can plot where from/to trucks are going
    // Public, it doesnt matter what touches this in the current state
    Position targetPosition;
    
private:
    int capacity = 0;
    int currentLoad = 0;
    TruckState state = TruckState::Idle;
};
