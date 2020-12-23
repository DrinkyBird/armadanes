#include "mappernrom.h"
#include "rom.h"

MapperNROM::MapperNROM(Rom *rom)
: Mapper(rom) {
    this->oneBank = rom->prgRomSize == 0x4000;
}

byte MapperNROM::readPrg(address addr) {
    if (!oneBank) {
        return this->rom->prgRom[addr];
    } else {
        return this->rom->prgRom[addr & 0x3FFF];
    }
}

byte MapperNROM::readChr(address addr) {
    return this->rom->chrRom[addr];
}