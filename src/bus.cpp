#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "bus.h"

Bus::Bus() {
    this->mappings = nullptr;
    this->numMappings = 0;
}

Bus::~Bus() {
    free(mappings);
}

void Bus::mapMemory(address start, address end, byte *region, address size) {
    BusMapping mapping;
    mapping.type = BusMappingType_Direct;
    mapping.startAddress = start;
    mapping.endAddress = end;
    mapping.direct.dest = region;
    mapping.direct.size = size;

    addMapping(&mapping);
}

void Bus::mapCallback(address start, address end, BusMapReadCallback readCallback, BusMapWriteCallback writeCallback, void *userData) {
    BusMapping mapping;
    mapping.type = BusMappingType_Callback;
    mapping.startAddress = start;
    mapping.endAddress = end;
    mapping.callback.readCallback = readCallback;
    mapping.callback.writeCallback = writeCallback;
    mapping.callback.userData = userData;

    addMapping(&mapping);
}

void Bus::addMapping(BusMapping *mapping) {
    size_t i = numMappings++;
    mappings = (BusMapping *)realloc(mappings, numMappings * sizeof(BusMapping));
    memcpy(&mappings[i], mapping, sizeof(BusMapping));
}

BusMapping *Bus::findMapping(address ptr, size_t *index) {
    for (size_t i = 0; i < numMappings; i++) {
        BusMapping *mapping = &mappings[i];
        if (ptr >= mapping->startAddress && ptr <= mapping->endAddress) {
            if (index != nullptr) {
                *index = i;
            }

            return mapping;
        }
    }

    return nullptr;
}

void Bus::resetMappings() {
    free(mappings);
    numMappings = 0;
    mappings = nullptr;
}

byte Bus::read(address ptr) {
    BusMapping *mapping = findMapping(ptr);
    if (!mapping) {
        return 0;
    }

    address offset = ptr - mapping->startAddress;

    switch (mapping->type) {
        case BusMappingType_Direct: {
            return mapping->direct.dest[offset % mapping->direct.size];
        }

        case BusMappingType_Callback: {
            return mapping->callback.readCallback(offset, mapping->callback.userData);
        }
    }

    return 0;
}

bool Bus::write(address ptr, byte value) {
    BusMapping *mapping = findMapping(ptr);
    if (!mapping) {
        return false;
    }

    address offset = ptr - mapping->startAddress;

    switch (mapping->type) {
        case BusMappingType_Direct: {
            mapping->direct.dest[offset % mapping->direct.size] = value;
            return true;
        }

        case BusMappingType_Callback: {
            return mapping->callback.writeCallback(offset, value, mapping->callback.userData);
        }
    }

    return false;
}

void Bus::dump() {
    FILE *f = fopen("bus.bin", "wb");

    for (size_t i = 0; i < 0x10000; i++) {
        byte data = read(i);
        fwrite(&data, 1, 1, f);
    }

    fclose(f);
}