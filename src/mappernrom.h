#pragma once

#include "mapper.h"

class MapperNROM : public Mapper {
public:
    MapperNROM(Rom *rom);
    ~MapperNROM() = default;

    byte readPrg(address addr) override;
    byte readChr(address addr) override;

private:
    bool oneBank;
};


