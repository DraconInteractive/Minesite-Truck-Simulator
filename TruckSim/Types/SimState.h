#pragma once

#include "../Entities/Mobile/Truck.h"
#include "../Entities/Stationary/Shovel.h"
#include "../Entities/Stationary/Dump.h"

struct CompareByTime {                                       
    bool operator()(const Event& a, const Event& b) {        
        return a.time > b.time;  // > makes it a min-heap  
    }                                                        
};

struct SimState
{
    std::vector<Truck>  trucks;
    std::vector<Shovel> shovels;
    std::vector<Dump>   dumps;
    std::priority_queue<Event, std::vector<Event>, CompareByTime> evtQueue;
    double currentTime = 0;
};
