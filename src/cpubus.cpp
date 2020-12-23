#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "cpubus.h"
#include "system.h"
#include "mapper.h"

const byte SPECIAL_UNMAPPED = 0xAF;
const byte SPECIAL_UNKNOWN_MAP_TYPE = 0xFA;

static byte readCartridgePrgRomCallback(address addr, void *userData);
static bool writeReadOnlyCallback(address addr, byte value, void *userData);

CpuBus::CpuBus(System *system) : Bus() {
    this->system = system;
    this->ram = new byte[0x800];

    mapMemory(0x0000, 0x1FFF, ram, 0x0800);
}

CpuBus::~CpuBus() {
    delete[] ram;
}

byte readCartridgePrgRomCallback(address addr, void *userData) {
    Mapper *mapper = (Mapper *)userData;
    return mapper->readPrg(addr);
}

bool writeReadOnlyCallback(address addr, byte value, void *userData) {
    return false;
}

void CpuBus::setCartridgeMapper(Mapper *mapper) {
    mapCallback(0x8000, 0xFFFF, readCartridgePrgRomCallback, writeReadOnlyCallback, mapper);
}