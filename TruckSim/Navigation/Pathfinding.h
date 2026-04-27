#pragma once

#include <cmath>
#include <unordered_map>

#include "../Entities/Entity.h"
#include "../Navigation/Node.h"
#include "../Navigation/Edge.h"
#include "../Types/SimState.h"

namespace Navigation
{
    inline Node GetNodeFromPosition(const SimState& sim, const float x, const float y)
    {
        float bestDist = std::numeric_limits<float>::max();
        const Node* best = sim.nodes.data();        
        for (const Node& n : sim.nodes)
        {
            float dx = n.x - x, dy = n.y - y;
            float d = dx*dx + dy*dy;
            if (d < bestDist) { bestDist = d; best = &n; }
        }
        return *best;
    }

    // Assume that nodeId for Node and Edge correspond directly to sim.nodes[x] and sim.edges[x]
    inline float GetTravelTimeById(const SimState& sim, int fromId, int toId, float truckSpeed)             
    {                                                                                                   
        std::unordered_map<int, float> dist;
        for (const Node& n : sim.nodes)                                                                 
            dist[n.id] = std::numeric_limits<float>::max();
        dist[fromId] = 0.f;                                                                             
                                                        
        using Entry = std::pair<float, int>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
        pq.push({ 0.f, fromId });

        while (!pq.empty())
        {
            float cost = pq.top().first;
            int u      = pq.top().second;
            pq.pop();

            if (cost > dist[u]) continue;
            if (u == toId) break;

            const Node& uNode = sim.nodes[u];

            for (int edgeId : uNode.connectedEdges)
            {
                const Edge& e = sim.edges[edgeId];
                int neighbour = (e.fromNode == u) ? e.toNode : e.fromNode;

                const Node& nNode = sim.nodes[neighbour];
                float dx = nNode.x - uNode.x, dy = nNode.y - uNode.y;
                float segDist = std::sqrt(dx*dx + dy*dy);
                float segTime = (segDist / (truckSpeed * e.speedMultiplier)) * 60.f;

                float newCost = dist[u] + segTime;
                if (newCost < dist[neighbour])
                {
                    dist[neighbour] = newCost;
                    pq.push({ newCost, neighbour });
                }
            }
        }

        return dist.count(toId) ? dist[toId] : std::numeric_limits<float>::max();
    }

    inline float GetTravelTimeByPosition(const SimState& sim, Position a, Position b, float speed)
    {
        const Node nodeA = GetNodeFromPosition(sim, a.x, a.y);
        const Node nodeB = GetNodeFromPosition(sim, b.x, b.y);

        return GetTravelTimeById(sim, nodeA.id, nodeB.id, speed);
    }
}
