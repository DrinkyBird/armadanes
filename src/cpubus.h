#pragma once

#include "bus.h"

class System;
class Mapper;

// represents the address space
class CpuBus : public Bus {
public:
    CpuBus(System *system);
    ~CpuBus();

    void setCartridgeMapper(Mapper *mapper);

private:
    System *system;

    byte *ram;
};


