#pragma once

class CpuBus;
class Rom;
class Cpu;
class Ppu;

class System {
public:
    System();
    ~System();

    bool loadRom(const char *path);

    void start();
    void reset();

    void tick();

    CpuBus *getBus() const { return this->bus; }
    Rom *getRom() const { return this->rom; }
    Cpu *getCpu() const { return this->cpu; }
    Ppu *getPpu() const { return this->ppu; }

private:
    CpuBus *bus;
    Rom *rom;
    Cpu *cpu;
    Ppu *ppu;
};


