#include "Simulation.h"

#include "Utilities.h"
#include "../Navigation/Pathfinding.h"

void Simulation::HandleTruckEnterSimulation (SimState& sim, const Event& evt)
{
    Truck& truck = sim.trucks[evt.truck.value];
    std::cout << "Truck entering simulation. Parts:\n";
    for (const EntityPart& part : truck.GetParts())
    {
        std::cout << " - " << part.name << "\n";
    }
    std::cout << "\n";
    // Will use predictive queue to find best shovel as it enters the sim
    DispatchTruckToShovel(sim, evt.truck);
}

void Simulation::HandleTruckArriveShovel(SimState& sim, const Event& evt)                           
{                                                                                                   
    Shovel& shovel = sim.shovels[evt.shovel.value];
    Truck& truck = sim.trucks[evt.truck.value];
    truck.SetPosition(shovel.GetPosition());
    truck.ClearPath();
    
    shovel.EnqueueTruck(evt.truck);

    if (shovel.TrucksInQueue() == 1) // Only queue event if first in line, trucks in line will trigger when first leaves queue
    {
        const float duration = shovel.TimeToProcess(truck) ;
        auto finishLoadingEvt = Event{sim.currentTime + duration, evt.truck, evt.shovel, {}, EventType::TruckFinishLoading};
        truck.SetState(TruckState::Loading);
        truck.StartTask(sim.currentTime, finishLoadingEvt);
        sim.evtQueue.push(finishLoadingEvt);
    }
    else
    {
        truck.SetState(TruckState::Queueing);
    }                                                                                        
}

void Simulation::HandleTruckFinishLoading(SimState& sim, const Event& evt)
{
    Shovel& shovel = sim.shovels[evt.shovel.value];
    Truck& truckLeavingShovel = sim.trucks[evt.truck.value];

    sim.totalUnitsLoaded += static_cast<float>(truckLeavingShovel.RemainingCapacity());
    truckLeavingShovel.Fill();
                    
    shovel.DequeueTruck(); // Truck has finished loading, and is leaving, so remove from queue

    // Move truck to dump
    DispatchTruckToDump(sim, evt.truck);

    // Process next truck in queue
    if (shovel.TrucksInQueue() > 0)
    {
        const TruckId nextTruckId = shovel.GetFirst();
        Truck& nextTruck = sim.trucks[nextTruckId.value];
        nextTruck.SetState(TruckState::Loading);
        float loadDuration = shovel.TimeToProcess(nextTruck);
        auto finishLoadingEvt = Event{sim.currentTime + loadDuration, nextTruckId, evt.shovel, {}, EventType::TruckFinishLoading};
        nextTruck.StartTask(sim.currentTime, finishLoadingEvt);
        sim.evtQueue.push(finishLoadingEvt);
    }
}

void Simulation::HandleTruckArriveDump(SimState& sim, const Event& evt)
{
    Dump& dump = sim.dumps[evt.dump.value];
    Truck& truck = sim.trucks[evt.truck.value];

    truck.SetPosition(dump.GetPosition());
    truck.ClearPath();
    
    dump.EnqueueTruck(evt.truck);

    if (dump.TrucksInQueue() == 1)
    {
        truck.SetState(TruckState::Dumping);
        float duration = dump.TimeToProcess(truck);
        auto finishDumpingEvt = Event{sim.currentTime + duration, evt.truck, {}, evt.dump, EventType::TruckFinishDumping};
        truck.StartTask(sim.currentTime, finishDumpingEvt);
        sim.evtQueue.push(finishDumpingEvt);
    }
    else
    {
        truck.SetState(TruckState::Queueing);
    }
}

void Simulation::HandleTruckFinishDumping (SimState& sim, const Event& evt)
{
    Dump& dump = sim.dumps[evt.dump.value];
    Truck& truckLeavingDump = sim.trucks[evt.truck.value];

    sim.totalUnitsDumped += static_cast<float>(truckLeavingDump.CurrentLoad());
    truckLeavingDump.Empty();
                    
    dump.DequeueTruck();

    DispatchTruckToShovel(sim, evt.truck);
                    
    // Start processing next truck in queue
    if (dump.TrucksInQueue() > 0)
    {
        TruckId nextTruckId = dump.GetFirst();
        Truck& nextTruck = sim.trucks[nextTruckId.value];
        nextTruck.SetState(TruckState::Dumping);
        float dumpDuration = dump.TimeToProcess(nextTruck);
        Event finishDumpEvtEvent{sim.currentTime + dumpDuration, nextTruckId, {}, evt.dump, EventType::TruckFinishDumping};
        nextTruck.StartTask(sim.currentTime, finishDumpEvtEvent);
        sim.evtQueue.push(finishDumpEvtEvent);
    }
}

void Simulation::HandleTruckPartFail(SimState& sim, const Event& evt)
{
    Truck& truck = sim.trucks[evt.truck.value];
    
    const EntityPart& part = truck.GetBrokenPart();
    Event fixedEvt = {sim.currentTime + part.repairTime, evt.truck, {}, {}, EventType::TruckPartFixed};
    truck.StartTask(sim.currentTime, fixedEvt);

    if (sim.debug.pauseOnFail)
    {
        sim.isPaused = true;
    }
    sim.evtQueue.push(fixedEvt);
}

void Simulation::HandleTruckPartFixed(SimState& sim, const Event& evt)
{
    Truck& truck = sim.trucks[evt.truck.value];

    truck.RepairBrokenPart();

    if (truck.CurrentLoad() > 0)
    {
        DispatchTruckToDump(sim, evt.truck);
    }
    else
    {
        DispatchTruckToShovel(sim, evt.truck);
    }
}

// Helpers
// Current issue with dispath: path will snap truck position to nearest node.
// This could be significant inaccuracy if truck is in middle of long edge
void Simulation::DispatchTruckToDump(SimState& sim, TruckId truckId)
{
    Truck& truck = sim.trucks[truckId.value];
    
    truck.SetState(TruckState::Travelling);
    
    const DumpId bestDump = Dump::GetBestDump(sim, truck);
    const Position dumpPos = sim.dumps[bestDump.value].GetPosition();
    const Navigation::PathResult navPath = Navigation::GetPathByPosition(sim, truck.GetPosition(), dumpPos, truck.GetSpeed());
    truck.targetPosition = dumpPos;
    truck.SetPath(navPath.nodes);
    
    int fail = truck.RollForFailure();
    if (fail != -1)
    {
        truck.SetState(TruckState::Broken);
        float progressBeforeFail = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float travelTimeBeforeFail = navPath.travelTime * progressBeforeFail;

        Position progressPosition = Navigation::GetPositionAlongPath(sim, navPath.nodes, progressBeforeFail, truck.GetSpeed());
        
        truck.SetPosition(progressPosition);
        truck.targetPosition = progressPosition;
        truck.ClearPath();
        
        auto truckFailedEvt = Event{sim.currentTime + travelTimeBeforeFail, truckId, {}, {}, EventType::TruckPartFail};
        truck.StartTask(sim.currentTime, truckFailedEvt);
        sim.evtQueue.push(truckFailedEvt);

        sim.totalTimeSpentTravelling += travelTimeBeforeFail;
        return;
    }

    sim.totalTimeSpentTravelling += navPath.travelTime;

    // Didnt fail, so apply wear to parts
    truck.ApplyWear();
    
    auto arriveAtDumpEvt = Event{sim.currentTime + navPath.travelTime, truckId, {}, bestDump, EventType::TruckArriveDump};
    truck.StartTask(sim.currentTime, arriveAtDumpEvt);

    sim.evtQueue.push(arriveAtDumpEvt);
}

void Simulation::DispatchTruckToShovel(SimState& sim, TruckId truckId)
{
    Truck& truck = sim.trucks[truckId.value];
    
    truck.SetState(TruckState::Travelling);
    
    ShovelId bestShovelId = Shovel::GetBestShovel(sim, truck);
    Position shovelPos = sim.shovels[bestShovelId.value].GetPosition();
    
    const Navigation::PathResult navPath = Navigation::GetPathByPosition(sim, truck.GetPosition(), shovelPos, truck.GetSpeed());
    truck.targetPosition = shovelPos;
    truck.SetPath(navPath.nodes);
    
    int fail = truck.RollForFailure();
    if (fail != -1)
    {
        truck.SetState(TruckState::Broken);
        float progressBeforeFail =static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float travelTimeBeforeFail = navPath.travelTime * progressBeforeFail;

        Position progressPosition = Navigation::GetPositionAlongPath(sim, navPath.nodes, progressBeforeFail, truck.GetSpeed());
        truck.SetPosition(progressPosition);
        truck.targetPosition = progressPosition;
        truck.ClearPath();

        auto truckFailedEvt = Event{sim.currentTime + travelTimeBeforeFail, truckId, {}, {}, EventType::TruckPartFail};
        truck.StartTask(sim.currentTime, truckFailedEvt);
        sim.evtQueue.push(truckFailedEvt);

        sim.totalTimeSpentTravelling += travelTimeBeforeFail;
        return;
    }

    sim.totalTimeSpentTravelling += navPath.travelTime;
    
    // Didn't fail so apply wear
    truck.ApplyWear();
    
    auto arriveAtShovelEvt = Event{sim.currentTime + navPath.travelTime, truckId, bestShovelId, {}, EventType::TruckArriveShovel};
    truck.StartTask(sim.currentTime, arriveAtShovelEvt);

    sim.evtQueue.push(arriveAtShovelEvt);
}

