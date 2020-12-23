#pragma once

#include "armadadef.h"

class System;

struct PpuRegisters {
    byte ppuctrl;
    byte ppumask;
    byte ppustatus;
    byte oamaddr;
    byte oamdata;
    byte ppuscroll;
    byte ppuaddr;
    byte ppudata;
};

class Ppu {
public:
    Ppu(System *system);
    ~Ppu();

    void step();

    PpuRegisters registers;

private:
    System *system;
};


