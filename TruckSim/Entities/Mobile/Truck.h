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

    void StartTask(float startTime, float duration)
    {
        timeTaskStarted = startTime;
        taskDuration = duration;
    }
    
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

    int RemainingCapacity() const
    {
        return capacity - currentLoad;
    }

    int CurrentLoad() const
    {
        return currentLoad;
    }
    
    int Fill()
    {
        currentLoad = capacity;
        return 0;
    }
    
    // Returns what doesnt fit
    int Fill(int amount)
    {
        currentLoad += amount;
        if (currentLoad > capacity)
        {
            int over = capacity - currentLoad;
            currentLoad = capacity;
            return over;
        }
        return 0;
    }

    int Empty()
    {
        currentLoad = 0;
        return 0;
    }
    
    // Returns what it couldnt empty (because there was none left)
    int Empty(int amount)
    {
        currentLoad -= amount;
        
        if (currentLoad < 0)
        {
            int under = std::abs(currentLoad);
            currentLoad = 0;
            return under;
        }

        return 0;
    }

    float EstTaskTimeRemaining(float currentTime) const
    {
        if (currentTime > timeTaskStarted + taskDuration)
        {
            return 0;
        }
        
        return timeTaskStarted + taskDuration - currentTime;
    }

    float EstTaskCompletionPercentage(float currentTime) const
    {
        if (taskDuration <= 0) return 1.0f;
        float t = (currentTime - timeTaskStarted) / taskDuration;
        return t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
    }
    
private:
    int capacity = 0;
    int currentLoad = 0;
    float timeTaskStarted = 0;
    float taskDuration = 0;
    TruckState state = TruckState::Idle;
};
