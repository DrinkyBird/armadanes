#include "ppubus.h"

PpuBus::PpuBus(System *system) : Bus() {
    this->system = system;
    this->ram = new byte[0x800];

    mapMemory(0x2000, 0x2FFF, ram, 0x0800);
}

PpuBus::~PpuBus() {
    delete[] ram;
}