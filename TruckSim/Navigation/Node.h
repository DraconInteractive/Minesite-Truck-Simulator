#pragma once
#include <vector>

struct Node
{
    int id;
    float x;
    float y;
    std::vector<int> connectedEdges;
};