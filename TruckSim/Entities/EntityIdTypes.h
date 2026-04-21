#pragma once

// Extracting these to their own file so that they can be referenced without referencing the full entity
struct TruckId
{
    int value = -1; // Default to invalid ID

    bool operator==(const TruckId& other) const
    {
        return value == other.value;
    }
};

struct DumpId
{
    int value = -1;

    bool operator==(const DumpId& other) const
    {
        return value == other.value;
    }
};

struct ShovelId
{
    int value = -1;

    bool operator==(const ShovelId& other) const
    {
        return value == other.value;
    }
};
