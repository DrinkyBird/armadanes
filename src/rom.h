#pragma once
#include <cstdint>
#include "armadadef.h"

class System;
class Mapper;

struct InesHeader {
    union {
        uint32_t magic32;
        char magic[4];
    };

    uint8_t prgRomSize;
    uint8_t chrRomSize;
    uint8_t flags6;
    uint8_t flags7;
    uint8_t flags8;
    uint8_t flags9;
    uint8_t flags10;
    uint8_t padding11, padding12, padding13, padding14, padding15;
};

enum {
    InesFlags6_UsesVerticalMirroring            = 1 << 0,
    InesFlags6_BatteryBackedPrgRam              = 1 << 1,
    InesFlags6_ContainsTrainer                  = 1 << 2,
    InesFlags6_IgnoreMirroringBit               = 1 << 3,
};

class Rom {
public:
    Rom(System *system);
    ~Rom();

    bool load(const char *path);

    byte *trainer;
    uint32_t trainerSize;
    byte *prgRom;
    uint32_t prgRomSize;
    byte *chrRom;
    uint32_t chrRomSize;

    int mapperNumber;
    Mapper *createMapper();

    void dump();

private:
    System *system;
};


