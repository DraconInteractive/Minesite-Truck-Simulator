#pragma once
#include "../Entities/EntityIdTypes.h"


enum class EventType : uint8_t
{
    TruckEnterSimulation,
    TruckArriveShovel,
    TruckFinishLoading,
    TruckArriveDump,
    TruckFinishDumping,
    TruckPartFail,
    TruckPartFixed
};

inline std::string EventTypeToString(EventType type)
{
    switch (type)
    {
    case EventType::TruckEnterSimulation:   return "TruckEnterSimulation";
    case EventType::TruckArriveShovel:      return "TruckArriveShovel";
    case EventType::TruckFinishLoading:     return "TruckFinishLoading";
    case EventType::TruckArriveDump:        return "TruckArriveDump";
    case EventType::TruckFinishDumping:     return "TruckFinishDumping";
    case EventType::TruckPartFail:          return "TruckPartFail";
    case EventType::TruckPartFixed:         return "TruckPartFixed";
    }
    return "Unknown";
}

struct Event
{
    float time;
    TruckId truck;
    ShovelId shovel;
    DumpId dump;
    EventType type;
    std::string debug;
};
