#pragma once
#include "armadadef.h"

class Cpu;

struct CpuRegisters {
    byte a;         // A (accumulator)
    byte x;         // X (index X)
    byte y;         // Y (index Y)
    uint16_t pc;    // PC (program counter)
    byte s;         // S (stack pointer)
    byte p;         // P (processor status)
};

enum {
    CpuStatusFlag_Carry                         = (1U << 0U), // 01
    CpuStatusFlag_Zero                          = (1U << 1U), // 02
    CpuStatusFlag_InterruptDisable              = (1U << 2U), // 04
    CpuStatusFlag_Decimal                       = (1U << 3U), // 08
    CpuStatusFlag_Break                         = (1U << 4U), // 10
    CpuStatusFlag_Constant                      = (1U << 5U), // 20
    CpuStatusFlag_Overflow                      = (1U << 6U), // 40
    CpuStatusFlag_Negative                      = (1U << 7U), // 80
};

struct CpuInstruction;
typedef address (Cpu::*AddressingCallback)(CpuInstruction *instruction);
typedef void (Cpu::*OpCallback)(CpuInstruction *instruction, address addr);

struct CpuInstruction {
    bool legal;
    byte opcode;
    AddressingCallback addressingMode;
    OpCallback operation;
    unsigned cycles;
    const char *addressingModeName;
    const char *operationName;
};

const address VECTOR_NMI = 0xFFFA;
const address VECTOR_RESET = 0xFFFC;
const address VECTOR_IRQ = 0xFFFE;