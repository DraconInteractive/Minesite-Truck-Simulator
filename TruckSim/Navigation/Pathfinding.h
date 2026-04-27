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

    struct PathResult { std::vector<int> nodes; float travelTime; };
    
    // Assume that nodeId for Node and Edge correspond directly to sim.nodes[x] and sim.edges[x]
    inline PathResult GetPathById(const SimState& sim, int fromId, int toId, float truckSpeed)             
    {                                                                                                   
        std::unordered_map<int, float> dist;
        std::unordered_map<int, int> prev;
        
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
                    prev[neighbour] = u;
                    pq.push({ newCost, neighbour });
                }
            }
        }

        if (!dist.count(toId) || dist[toId] == std::numeric_limits<float>::max())
            return { {}, std::numeric_limits<float>::max() };

        // Reconstruct path by walking prev backwards
        std::vector<int> path;
        for (int cur = toId; cur != fromId; cur = prev[cur])
            path.push_back(cur);
        path.push_back(fromId);
        std::reverse(path.begin(), path.end());

        return { path, dist[toId] };
    }

    inline PathResult GetPathByPosition(const SimState& sim, Position a, Position b, float speed)
    {
        const Node nodeA = GetNodeFromPosition(sim, a.x, a.y);
        const Node nodeB = GetNodeFromPosition(sim, b.x, b.y);

        return GetPathById(sim, nodeA.id, nodeB.id, speed);
    }

    inline int GetEdgeToNeighbour(const SimState& sim, int fromNode, int toNode)                               
    {                                                                                                   
        for (int edgeId : sim.nodes[fromNode].connectedEdges)                                           
        {                                                                                               
            const Edge& e = sim.edges[edgeId];                                                          
            if (e.fromNode == toNode || e.toNode == toNode)                                             
                return edgeId;                            
        }
        return -1; // no direct edge
    }


    inline Position GetPositionAlongPath(const SimState& sim, const std::vector<int>& pathNodes, float t, float truckSpeed)                                      
    {
        if (pathNodes.empty())
            return {0.f, 0.f};
        
        if (pathNodes.size() == 1)                                                                      
            return { sim.nodes[pathNodes[0]].x, sim.nodes[pathNodes[0]].y };
                                                                                                      
        // Compute total path time and per-segment times                                          
        std::vector<float> segTimes;                                                                  
        float totalTime = 0.f;                                                                          
        for (int i = 0; i < (int)pathNodes.size() - 1; i++)                                             
        {
            const Node& a = sim.nodes[pathNodes[i]];
            const Node& b = sim.nodes[pathNodes[i + 1]];
            const Edge& e = sim.edges[GetEdgeToNeighbour(sim, a.id, b.id)];
            float dx = b.x - a.x, dy = b.y - a.y;
            float len = std::sqrt(dx*dx + dy*dy);
            float segTime = (len / (truckSpeed * e.speedMultiplier))* 60.f;
            segTimes.push_back(segTime);
            totalTime += segTime;
        }

        // Find which segment t falls in
        float target = t * totalTime;
        float accumulated = 0.f;
        for (int i = 0; i < (int)segTimes.size(); i++)
        {
            if (accumulated + segTimes[i] >= target)
            {
                float segT = (target - accumulated) / segTimes[i];
                const Node& a = sim.nodes[pathNodes[i]];
                const Node& b = sim.nodes[pathNodes[i + 1]];
                return { a.x + (b.x - a.x) * segT, a.y + (b.y - a.y) * segT };
            }
            accumulated += segTimes[i];
        }

        // t >= 1, return destination
        const Node& last = sim.nodes[pathNodes.back()];
        return { last.x, last.y };
    }
}
