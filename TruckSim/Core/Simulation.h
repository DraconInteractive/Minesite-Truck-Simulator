#pragma once

#include "../Types/SimState.h"

namespace Simulation                                                                                
{                                       
    void HandleTruckArriveShovel    (SimState& sim, const Event& evt);                                
    void HandleTruckFinishLoading   (SimState& sim, const Event& evt);                                
    void HandleTruckArriveDump      (SimState& sim, const Event& evt);                                
    void HandleTruckFinishDumping   (SimState& sim, const Event& evt);

    void DispatchTruckToDump        (SimState& sim, TruckId truckId);
    void DispatchTruckToShovel      (SimState& sim, TruckId truckId);
} 
