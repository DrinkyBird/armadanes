#pragma once

#include "armadadef.h"

class Rom;
class CpuBus;

class Mapper {
public:
    Mapper(Rom *rom);
    virtual ~Mapper() = default;

    virtual byte readPrg(address addr) = 0;
    virtual byte readChr(address addr) = 0;

protected:
    Rom *rom;
};


