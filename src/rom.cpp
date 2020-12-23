#include <cstdio>
#include <cstring>
#include "rom.h"
#include "system.h"
#include "mapper.h"
#include "mappernrom.h"

Rom::Rom(System *system) {
    this->system = system;
    this->trainer = nullptr;
    this->trainerSize = 0;
    this->prgRom = nullptr;
    this->prgRomSize = 0;
    this->chrRom = nullptr;
    this->chrRomSize = 0;
}

Rom::~Rom() {
    delete[] this->chrRom;
    delete[] this->prgRom;
    delete[] this->trainer;
}

bool Rom::load(const char *path) {
    InesHeader header;

    printf("Load %s\n", path);

    FILE *f = fopen(path, "rb");
    fread(&header, sizeof(InesHeader), 1, f);

    if (memcmp(header.magic, "NES\x1A", 4) != 0) {
        fclose(f);
        return false;
    }

    bool verticalMirroring = header.flags6 & InesFlags6_UsesVerticalMirroring;
    bool batteryPrgRam = header.flags6 & InesFlags6_BatteryBackedPrgRam;
    bool containsTrainer = header.flags6 & InesFlags6_ContainsTrainer;
    bool ignoreMirroringBit = header.flags6 & InesFlags6_IgnoreMirroringBit;
    mapperNumber = ((header.flags6 >> 4) & 0xF) | (header.flags7 & 0xF0);

    trainerSize = containsTrainer ? 512 : 0;
    if (trainerSize != 0) {
        trainer = new byte[trainerSize];
        fread(trainer, trainerSize, 1, f);
    }

    prgRomSize = header.prgRomSize * 16384;
    prgRom = new byte[prgRomSize];
    fread(prgRom, prgRomSize, 1, f);

    chrRomSize = header.chrRomSize * 8192;
    chrRom = new byte[chrRomSize];
    fread(chrRom, chrRomSize, 1, f);

    fclose(f);

    return true;
}

void Rom::dump() {
    FILE *f;

    f = fopen("trainer.bin", "wb");
    fwrite(trainer, 1, trainerSize, f);
    fclose(f);

    f = fopen("prgrom.bin", "wb");
    fwrite(prgRom, 1, prgRomSize, f);
    fclose(f);

    f = fopen("chrrom.bin", "wb");
    fwrite(chrRom, 1, chrRomSize, f);
    fclose(f);
}

Mapper *Rom::createMapper() {
    switch (mapperNumber) {
        case 0x00: {
            return new MapperNROM(this);
        }
    }

    return nullptr;
}