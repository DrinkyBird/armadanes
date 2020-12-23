#include <cstring>
#include "ppu.h"
#include "system.h"

Ppu::Ppu(System *system) {
    this->system = system;
    memset(&this->registers, 0, sizeof(PpuRegisters));
}

Ppu::~Ppu() {

}
