#include "Simulation.h"

#include "Utilities.h"

void Simulation::HandleTruckEnterSimulation (SimState& sim, const Event& evt)
{
    // Will use predictive queue to find best shovel as it enters the sim
    DispatchTruckToShovel(sim, evt.truck);
}

void Simulation::HandleTruckArriveShovel(SimState& sim, const Event& evt)                           
{                                                                                                   
    Shovel& shovel = sim.shovels[evt.shovel.value];
    Truck& truck = sim.trucks[evt.truck.value];
    truck.SetPosition(shovel.GetPosition());
                    
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

// Helpers

void Simulation::DispatchTruckToDump(SimState& sim, TruckId truckId)
{
    Truck& truck = sim.trucks[truckId.value];
    
    truck.SetState(TruckState::Travelling);
    
    const DumpId bestDump = Dump::GetBestDump(sim, truck);
    const Position dumpPos = sim.dumps[bestDump.value].GetPosition();
    const float travelTimeToDump = Utilities::GetTravelTime(truck.GetPosition(), dumpPos, truck.GetSpeed());
    truck.targetPosition = dumpPos;

    auto arriveAtDumpEvt = Event{sim.currentTime + travelTimeToDump, truckId, {}, bestDump, EventType::TruckArriveDump};
    truck.StartTask(sim.currentTime, arriveAtDumpEvt);

    sim.evtQueue.push(arriveAtDumpEvt);
}

void Simulation::DispatchTruckToShovel(SimState& sim, TruckId truckId)
{
    Truck& truck = sim.trucks[truckId.value];
    
    truck.SetState(TruckState::Travelling);
    
    ShovelId bestShovelId = Shovel::GetBestShovel(sim, truck);
    const float travelTime = Utilities::GetTravelTime(truck.GetPosition(), sim.shovels[bestShovelId.value].GetPosition(), truck.GetSpeed());
    truck.targetPosition = sim.shovels[bestShovelId.value].GetPosition();

    auto arriveAtShovelEvt = Event{sim.currentTime + travelTime, truckId, bestShovelId, {}, EventType::TruckArriveShovel};
    truck.StartTask(sim.currentTime, arriveAtShovelEvt);

    sim.evtQueue.push(arriveAtShovelEvt);
}