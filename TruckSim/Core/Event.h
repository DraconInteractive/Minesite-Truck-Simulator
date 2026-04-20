#pragma once
#include "../Entities/EntityIdTypes.h"


enum class EventType : uint8_t
{
    TruckArriveShovel,
    TruckFinishLoading,
    TruckArriveDump,
    TruckFinishDumping
};

inline std::string EventTypeToString(EventType type)
{
    switch (type)
    {
    case EventType::TruckArriveShovel:      return "TruckArriveShovel";
    case EventType::TruckFinishLoading:     return "TruckFinishLoading";
    case EventType::TruckArriveDump:        return "TruckArriveDump";
    case EventType::TruckFinishDumping:     return "TruckFinishDumping";
    }
}

struct Event
{
    double time;
    TruckId truck;
    ShovelId shovel;
    DumpId dump;
    EventType type;
};