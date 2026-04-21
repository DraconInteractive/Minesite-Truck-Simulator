#include "Config.h"
#include <fstream>                                                                           
#include <stdexcept>                                                                         
#include <nlohmann/json.hpp>                                                                 
using json = nlohmann::json;

Config Config::Load(const std::string& path)                                                  
{                                                                                             
   std::ifstream file(path);                                                                 
   if (!file.is_open())                                                                      
       throw std::runtime_error("Could not open config file: " + path);                      
                                                                                             
   json j = json::parse(file);                                                               
   Config cfg;                                                                               
                                                                                             
   for (auto& t : j["trucks"])                                                               
       cfg.trucks.push_back({ t["id"], t["ladenSpeed"], t["unladenSpeed"], t["capacity"] });                         
                                                                                             
   for (auto& s : j["shovels"])                                                              
       cfg.shovels.push_back({ s["id"], s["x"], s["y"], s["loadSpeed"] });                   
                                                                                             
   for (auto& d : j["dumps"])                                                                
       cfg.dumps.push_back({ d["id"], d["x"], d["y"], d["dumpSpeed"] });                     
                                                                                             
   for (auto& e : j["seedEvents"])                                                           
       cfg.seedEvents.push_back({ e["truckId"], e["shovelId"], e["arrivalTime"] });          
                                                                                             
   // Optional blocks — use defaults from struct if absent                                   
   if (j.contains("routing"))                                                                
   {                                                                                         
       cfg.routing.distancePriority = j["routing"].value("distancePriority", 1.f);           
       cfg.routing.queuePriority    = j["routing"].value("queuePriority",    1.f);           
   }                                                                                         
                                                                                             
   if (j.contains("debug"))                                                                  
   {                                                                                         
       cfg.debug.logEvents = j["debug"].value("logEvents", true);                            
       cfg.debug.timeCap   = j["debug"].value("timeCap",   1000.f);                          
   }                                                                                         
                                                                                             
   return cfg;                                                                               
}

SimState Config::BuildSimState(const Config& cfg)
{
    SimState sim;
    sim.routing = cfg.routing;

    for (auto& t : cfg.trucks)
        sim.trucks.emplace_back(t.id, t.ladenSpeed, t.unladenSpeed, t.capacity, 0);

    for (auto& s : cfg.shovels)
        sim.shovels.emplace_back(s.id, Position{s.x, s.y}, s.loadSpeed);

    for (auto& d : cfg.dumps)
        sim.dumps.emplace_back(d.id, Position{d.x, d.y}, d.dumpSpeed);

    for (auto& e : cfg.seedEvents)
    {
        Event evt{e.arrivalTime, TruckId{e.truckId}, ShovelId{e.shovelId}, {}, EventType::TruckEnterSimulation};
        sim.trucks[e.truckId].StartTask(0, evt);
        sim.trucks[e.truckId].SetState(TruckState::Idle); // Set truck to idle before entry event is processed
        sim.evtQueue.push(evt);
    }

    return sim;
}
