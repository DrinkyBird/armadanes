#include <cstdio>
#include "system.h"
#include "rom.h"
#include "cpubus.h"
#include "cpu.h"
#include "ppu.h"

System::System() {
    this->bus = new CpuBus(this);
    this->cpu = new Cpu(this);
    this->ppu = new Ppu(this);
};

System::~System() {
    delete ppu;
    delete cpu;
    delete bus;
}

bool System::loadRom(const char *path) {
    this->rom = new Rom(this);
    bool v = rom->load(path);
    Mapper *mapper = this->rom->createMapper();
    this->bus->setCartridgeMapper(mapper);
    return v;
}

void System::start() {
    bus->mapMemory(0x2000, 0x4000, (byte *)&ppu->registers, sizeof(PpuRegisters));

    cpu->start();
    printf("Started\n");
}

void System::reset() {
    cpu->reset();
    printf("Reset\n");
}

void System::tick() {
    cpu->step();
}