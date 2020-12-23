#pragma once

#include "bus.h"

class System;
class Mapper;

class PpuBus : public Bus {
public:
    PpuBus(System *system);
    ~PpuBus();

    void setCartridgeMapper(Mapper *mapper);

private:
    System *system;

    byte *ram;
};


