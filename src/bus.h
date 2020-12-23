#pragma once

#include "armadadef.h"

enum {
    // simply maps emulated memory region to host memory region
    BusMappingType_Direct,
    // uses a callback to map
    BusMappingType_Callback,
};

// Function pointer type definitions for callback mappings.
// offset is relative to the mapping, not the absolute address
typedef byte (*BusMapReadCallback)(address offset, void *userdata);
typedef bool (*BusMapWriteCallback)(address offset, byte value, void *userdata);

struct BusMapping {
    int type;
    address startAddress;
    address endAddress;

    union {
        struct {
            byte *dest;
            address size;
        } direct;

        struct {
            BusMapReadCallback readCallback;
            BusMapWriteCallback writeCallback;
            void *userData;
        } callback;
    };
};

class Bus {
public:
    Bus();
    virtual ~Bus();

    byte read(address ptr);
    bool write(address ptr, byte value);

    void mapMemory(address start, address end, byte *region, address size);
    void mapCallback(address start, address end, BusMapReadCallback readCallback, BusMapWriteCallback writeCallback, void *userData = nullptr);

    void dump();
private:
    void addMapping(BusMapping *mapping);
    BusMapping *findMapping(address ptr, size_t *index = nullptr);
    void resetMappings();

    BusMapping *mappings;
    size_t numMappings;
};
