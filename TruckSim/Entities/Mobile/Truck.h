#pragma once
#include <iostream>
#include <vector>

#include "MobileEntity.h"
#include "../../Core/Event.h"
#include "../../Core/Utilities.h"
#include "../../Types/EntityPart.h"

enum class TruckState
{
    Idle,
    Travelling,
    Loading,
    Queueing,
    Dumping,
    Broken
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
        case TruckState::Broken:     return "Broken";
    }
    return "Unknown";
}

class Truck : public MobileEntity
{
public:
    Truck(int id_, float ladenSpeed, float unladenSpeed, int capacity, int currentLoad) : MobileEntity(id_), capacity(capacity), currentLoad(currentLoad), ladenSpeed(ladenSpeed), unladenSpeed(unladenSpeed) {} // TODO make a sort of construction DTO so we can create an entity from a config

    void StartTask(float startTime, Event scheduledEvent)
    {
        timeTaskStarted = startTime;
        nextEvent = scheduledEvent;
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
    Position targetPosition = {0,0};

    float GetSpeed() const override
    {
        return Utilities::Lerp(unladenSpeed, ladenSpeed, static_cast<float>(currentLoad) / static_cast<float>(capacity));
    }
    
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
        return nextEvent.time - currentTime;
    }

    float EstTaskCompletionPercentage(float currentTime) const
    {
        const float total = nextEvent.time - timeTaskStarted;
        if (total <= 0) return 1.0f;
        const float t = (currentTime - timeTaskStarted) / total;
        return t < 0.f ? 0.f : t > 1.f ? 1.f : t;
    }

    Event GetNextEvent() const
    {
        return nextEvent;
    }

    bool IsBroken() const
    {
        return brokenPartIndex >= 0;
    }
    
    const EntityPart& GetBrokenPart() const
    {
        return parts[brokenPartIndex];
    }

    void ApplyWear()
    {
        for (auto& p : parts)
            p.health = std::max(0.0f, p.health - p.wearPerCycle);
    }
    
    int RollForFailure();
    
    void RepairBrokenPart()
    {
        parts[brokenPartIndex].health = 1.0f;
        brokenPartIndex = -1;
    }

    const std::vector<EntityPart>& GetParts() const
    {
        return parts;    
    }
    
    void SetParts(std::vector<EntityPart> newParts)
    {
        parts = std::move(newParts);
    }

private:
    int capacity = 0;
    int currentLoad = 0;
    float ladenSpeed = 0;
    float unladenSpeed = 0;
    
    float timeTaskStarted = 0;
    Event nextEvent = {};

    std::vector<EntityPart> parts;
    int brokenPartIndex = -1;
    
    TruckState state = TruckState::Idle;
};
