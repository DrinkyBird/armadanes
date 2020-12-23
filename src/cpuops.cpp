// Implementation of CPU addressing modes and opcodes

#include <cstring>
#include "cpu.h"
#include "system.h"
#include "cpubus.h"

#define DEFINE_ADDRESS_MODE(mnemonic) \
    address Cpu::addr##mnemonic(CpuInstruction *instruction)

#define DEFINE_OPERATION(mnemonic) \
    void Cpu::op##mnemonic(CpuInstruction *instruction, address addr)

// read a single byte from memory
#define READ(addr) (this->system->getBus()->read(addr))
// write a single byte to memory
#define WRITE(addr, value) (this->system->getBus()->write(addr, value))
#define R_A (this->registers.a)
#define R_X (this->registers.x)
#define R_Y (this->registers.y)
#define R_PC (this->registers.pc)
#define R_S (this->registers.s)
#define R_P (this->registers.p)
#define F_C (R_P & CpuStatusFlag_Carry)
#define F_Z (R_P & CpuStatusFlag_Zero)
#define F_I (R_P & CpuStatusFlag_InterruptDisable)
#define F_D (R_P & CpuStatusFlag_Decimal)
#define F_V (R_P & CpuStatusFlag_Overflow)
#define F_N (R_P & CpuStatusFlag_Negative)
// set a flag in the P register to the result of a boolean expression
#define FLAG_SET(flag, value)       \
    if (!!(value)) {                \
        R_P |= flag;                \
    } else {                        \
        R_P &= ~flag;               \
    }
// set the Zero and Negative flags according to the given value
#define SET_ZN(value)                                   \
    FLAG_SET(CpuStatusFlag_Zero, (value) == 0x00);      \
    FLAG_SET(CpuStatusFlag_Negative, (value) & 0x80);

#define SPEND_CYCLES(numCyclesToSpend)                                  \
    if (numCyclesToSpend > 0) {                                         \
        this->cyclesToSkip += static_cast<unsigned>(numCyclesToSpend);  \
    }

#define SPEND_IF_PAGE_CROSSED(old, new, cycles)     \
    if (((old) & 0xFF00) != ((new) & 0xFF00)) {     \
        SPEND_CYCLES(cycles);                       \
    }

void Cpu::setupInstructions() {
    // initially define all opcodes as illegal
    for (byte i = 0; i < 0xFF; i++) {
        memset(&instructions[i], 0, sizeof(CpuInstruction));
        instructions[i].legal = false;
        instructions[i].opcode = i;
        instructions[i].cycles = 1;
    }

#define DEFINE_INST(_opcode, _addressingMode, _operation, _cycles)              \
    /* int multiple_decl_##_opcode; *//* prevents defining same opcode twice */ \
    /* int multiple_decl_##_addressingMode##_##_operation; */                   \
    instructions[(_opcode)].legal = true;                                       \
    instructions[(_opcode)].opcode = (_opcode);                                 \
    instructions[(_opcode)].addressingMode = &Cpu::addr##_addressingMode;       \
    instructions[(_opcode)].operation = &Cpu::op##_operation;                   \
    instructions[(_opcode)].cycles = _cycles;                                   \
    instructions[(_opcode)].addressingModeName = #_addressingMode;              \
    instructions[(_opcode)].operationName = #_operation;

    // There's no doubt a better way to do this, but being explicit is probably good when the emulator is this young

    // ADC
    DEFINE_INST(0x69, Imm, ADC, 2);
    DEFINE_INST(0x65, Zer, ADC, 3);
    DEFINE_INST(0x75, Zex, ADC, 4);
    DEFINE_INST(0x6D, Abs, ADC, 4);
    DEFINE_INST(0x7D, Abx, ADC, 4);
    DEFINE_INST(0x79, Aby, ADC, 4);
    DEFINE_INST(0x61, Inx, ADC, 6);
    DEFINE_INST(0x71, Iny, ADC, 5);

    // AND
    DEFINE_INST(0x29, Imm, AND, 2);
    DEFINE_INST(0x25, Zer, AND, 3);
    DEFINE_INST(0x35, Zex, AND, 4);
    DEFINE_INST(0x2D, Abs, AND, 4);
    DEFINE_INST(0x3D, Abx, AND, 4);
    DEFINE_INST(0x39, Aby, AND, 4);
    DEFINE_INST(0x21, Inx, AND, 6);
    DEFINE_INST(0x31, Iny, AND, 5);

    // ASL
    DEFINE_INST(0x0A, Acc, ASL_acc, 2);
    DEFINE_INST(0x06, Zer, ASL, 5);
    DEFINE_INST(0x16, Zex, ASL, 6);
    DEFINE_INST(0x0E, Abs, ASL, 6);
    DEFINE_INST(0x1E, Abx, ASL, 7);

    // BCC
    DEFINE_INST(0x90, Rel, BCC, 2);

    // BCS
    DEFINE_INST(0xB0, Rel, BCS, 2);

    // BEQ
    DEFINE_INST(0xF0, Rel, BEQ, 2);

    // BIT
    DEFINE_INST(0x24, Zer, BIT, 3);
    DEFINE_INST(0x2C, Abs, BIT, 4);

    // BMI
    DEFINE_INST(0x30, Rel, BMI, 2);

    // BNE
    DEFINE_INST(0xD0, Rel, BNE, 2);

    // BPL
    DEFINE_INST(0x10, Rel, BPL, 2);

    // BRK
    DEFINE_INST(0x00, Imp, BRK, 7);

    // BVC
    DEFINE_INST(0x50, Rel, BVC, 2);

    // BVS
    DEFINE_INST(0x70, Rel, BVS, 2);

    // CLC
    DEFINE_INST(0x18, Imp, CLC, 2);

    // CLD
    DEFINE_INST(0xD8, Imp, CLD, 2);

    // CLI
    DEFINE_INST(0x58, Imp, CLI, 2);

    // CLV
    DEFINE_INST(0xB8, Imp, CLV, 2);

    // CMP
    DEFINE_INST(0xC9, Imm, CMP, 2);
    DEFINE_INST(0xC5, Zer, CMP, 3);
    DEFINE_INST(0xD5, Zex, CMP, 4);
    DEFINE_INST(0xCD, Abs, CMP, 4);
    DEFINE_INST(0xDD, Abx, CMP, 4);
    DEFINE_INST(0xD9, Aby, CMP, 4);
    DEFINE_INST(0xC1, Inx, CMP, 6);
    DEFINE_INST(0xD1, Iny, CMP, 5);

    // CPX
    DEFINE_INST(0xE0, Imm, CPX, 2);
    DEFINE_INST(0xE4, Zer, CPX, 3);
    DEFINE_INST(0xEC, Abs, CPX, 4);

    // CPY
    DEFINE_INST(0xC0, Imm, CPY, 2);
    DEFINE_INST(0xC4, Zer, CPY, 3);
    DEFINE_INST(0xCC, Abs, CPY, 4);

    // DEC
    DEFINE_INST(0xC6, Zer, DEC, 5);
    DEFINE_INST(0xD6, Zex, DEC, 6);
    DEFINE_INST(0xCE, Abs, DEC, 6);
    DEFINE_INST(0xDE, Abx, DEC, 7);

    // DEX
    DEFINE_INST(0xCA, Imp, DEX, 2);

    // DEY
    DEFINE_INST(0x88, Imp, DEY, 2);

    // EOR
    DEFINE_INST(0x49, Imm, EOR, 2);
    DEFINE_INST(0x45, Zer, EOR, 3);
    DEFINE_INST(0x55, Zex, EOR, 4);
    DEFINE_INST(0x4D, Abs, EOR, 4);
    DEFINE_INST(0x5D, Abx, EOR, 4);
    DEFINE_INST(0x59, Aby, EOR, 4);
    DEFINE_INST(0x41, Inx, EOR, 6);
    DEFINE_INST(0x51, Iny, EOR, 5);

    // INC
    DEFINE_INST(0xE6, Zer, INC, 5);
    DEFINE_INST(0xF6, Zex, INC, 6);
    DEFINE_INST(0xEE, Abs, INC, 6);
    DEFINE_INST(0xFE, Abx, INC, 7);

    // INX
    DEFINE_INST(0xE8, Imp, INX, 2);

    // INY
    DEFINE_INST(0xC8, Imp, INY, 2);

    // JMP
    DEFINE_INST(0x4C, Abs, JMP, 3);
    DEFINE_INST(0x6C, Abi, JMP, 5);

    // JSR
    DEFINE_INST(0x20, Abs, JSR, 6);

    // LDA
    DEFINE_INST(0xA9, Imm, LDA, 2);
    DEFINE_INST(0xA5, Zer, LDA, 3);
    DEFINE_INST(0xB5, Zex, LDA, 4);
    DEFINE_INST(0xAD, Abs, LDA, 4);
    DEFINE_INST(0xBD, Abx, LDA, 4);
    DEFINE_INST(0xB9, Aby, LDA, 4);
    DEFINE_INST(0xA1, Inx, LDA, 6);
    DEFINE_INST(0xB1, Iny, LDA, 5);

    // LDX
    DEFINE_INST(0xA2, Imm, LDX, 2);
    DEFINE_INST(0xA6, Zer, LDX, 3);
    DEFINE_INST(0xB6, Zey, LDX, 4);
    DEFINE_INST(0xAE, Abs, LDX, 4);
    DEFINE_INST(0xBE, Aby, LDX, 4);

    // LDY
    DEFINE_INST(0xA0, Imm, LDY, 2);
    DEFINE_INST(0xA4, Zer, LDY, 3);
    DEFINE_INST(0xB4, Zex, LDY, 4);
    DEFINE_INST(0xAC, Abs, LDY, 4);
    DEFINE_INST(0xBC, Abx, LDY, 4);

    // LSR
    DEFINE_INST(0x4A, Acc, LSR_acc, 2);
    DEFINE_INST(0x46, Zer, LSR, 5);
    DEFINE_INST(0x56, Zex, LSR, 6);
    DEFINE_INST(0x4E, Abs, LSR, 6);
    DEFINE_INST(0x5E, Abx, LSR, 7);

    // NOP
    DEFINE_INST(0xEA, Imp, NOP, 2);

    // ORA
    DEFINE_INST(0x09, Imm, ORA, 2);
    DEFINE_INST(0x05, Zer, ORA, 3);
    DEFINE_INST(0x15, Zex, ORA, 4);
    DEFINE_INST(0x0D, Abs, ORA, 4);
    DEFINE_INST(0x1D, Abx, ORA, 4);
    DEFINE_INST(0x19, Aby, ORA, 4);
    DEFINE_INST(0x01, Inx, ORA, 6);
    DEFINE_INST(0x11, Iny, ORA, 5);

    // PHA
    DEFINE_INST(0x48, Imp, PHA, 3);

    // PHP
    DEFINE_INST(0x08, Imp, PHP, 3);

    // PLA
    DEFINE_INST(0x68, Imp, PLA, 4);

    // PLP
    DEFINE_INST(0x28, Imp, PLP, 4);

    // ROL
    DEFINE_INST(0x2A, Acc, ROL_acc, 2);
    DEFINE_INST(0x26, Zer, ROL, 5);
    DEFINE_INST(0x36, Zex, ROL, 6);
    DEFINE_INST(0x2E, Abs, ROL, 6);
    DEFINE_INST(0x3E, Abx, ROL, 7);

    // ROR
    DEFINE_INST(0x6A, Acc, ROR_acc, 2);
    DEFINE_INST(0x66, Zer, ROR, 5);
    DEFINE_INST(0x76, Zex, ROR, 6);
    DEFINE_INST(0x6E, Abs, ROR, 6);
    DEFINE_INST(0x7E, Abx, ROR, 7);

    // RTI
    DEFINE_INST(0x40, Imp, RTI, 6);

    // RTS
    DEFINE_INST(0x60, Imp, RTS, 6);

    // SBC
    DEFINE_INST(0xE9, Imm, SBC, 2);
    DEFINE_INST(0xE5, Zer, SBC, 3);
    DEFINE_INST(0xF5, Zex, SBC, 4);
    DEFINE_INST(0xED, Abs, SBC, 4);
    DEFINE_INST(0xFD, Abx, SBC, 4);
    DEFINE_INST(0xF9, Aby, SBC, 4);
    DEFINE_INST(0xE1, Inx, SBC, 6);
    DEFINE_INST(0xF1, Iny, SBC, 5);

    // SEC
    DEFINE_INST(0x38, Imp, SEC, 2);

    // SED
    DEFINE_INST(0xF8, Imp, SED, 2);

    // SEI
    DEFINE_INST(0x78, Imp, SEI, 2);

    // STA
    DEFINE_INST(0x85, Zer, STA, 3);
    DEFINE_INST(0x95, Zex, STA, 4);
    DEFINE_INST(0x8D, Abs, STA, 4);
    DEFINE_INST(0x9D, Abx, STA, 5);
    DEFINE_INST(0x99, Aby, STA, 5);
    DEFINE_INST(0x81, Inx, STA, 6);
    DEFINE_INST(0x91, Iny, STA, 6);

    // STX
    DEFINE_INST(0x86, Zer, STX, 3);
    DEFINE_INST(0x96, Zey, STX, 4);
    DEFINE_INST(0x8E, Abs, STX, 4);

    // STY
    DEFINE_INST(0x84, Zer, STY, 3);
    DEFINE_INST(0x94, Zex, STY, 4);
    DEFINE_INST(0x8C, Abs, STY, 4);

    // TAX
    DEFINE_INST(0xAA, Imp, TAX, 2);

    // TAY
    DEFINE_INST(0xA8, Imp, TAY, 2);

    // TSX
    DEFINE_INST(0xBA, Imp, TSX, 2);

    // TXA
    DEFINE_INST(0x8A, Imp, TXA, 2);

    // TXS
    DEFINE_INST(0x9A, Imp, TXS, 2);

    // TYA
    DEFINE_INST(0x98, Imp, TYA, 2);

#undef DEFINE_INST
}

DEFINE_ADDRESS_MODE(Acc) {
    return ZERO_ADDRESS;
}

DEFINE_ADDRESS_MODE(Imm) {
    return registers.pc++;
}

DEFINE_ADDRESS_MODE(Abs) {
    uint16_t addr = readAddress(R_PC);
    R_PC += 2;
    return addr;
}

DEFINE_ADDRESS_MODE(Abx) {
    uint16_t addr = readAddress(R_PC);
    R_PC += 2;
    if (strcmp(instruction->operationName, "STA") != 0) {
        SPEND_IF_PAGE_CROSSED(addr, addr + R_X, 1);
    }
    return addr + R_X;
}

DEFINE_ADDRESS_MODE(Aby) {
    uint16_t addr = readAddress(R_PC);
    R_PC += 2;
    if (strcmp(instruction->operationName, "STA") != 0) {
        SPEND_IF_PAGE_CROSSED(addr, addr + R_Y, 1);
    }
    return addr + R_Y;
}

DEFINE_ADDRESS_MODE(Zer) {
    return READ(R_PC++);
}

DEFINE_ADDRESS_MODE(Zex) {
    return (READ(R_PC++) + R_X) & 0xFF;
}

DEFINE_ADDRESS_MODE(Zey) {
    return (READ(R_PC++) + R_Y) & 0xFF;
}

DEFINE_ADDRESS_MODE(Imp) {
    return ZERO_ADDRESS;
}

DEFINE_ADDRESS_MODE(Rel) {
    address offset = READ(R_PC++);
    if (offset & 0x80) {
        offset |= 0xFF00;
    }

    address addr = R_PC + offset;
    return addr;
}

DEFINE_ADDRESS_MODE(Inx) {
    address zeroL = (READ(R_PC++) + R_X) & 0xFF;
    address zeroH = (zeroL + 1) & 0xFF;
    return READ(zeroL) | (READ(zeroH) << 8);
}

DEFINE_ADDRESS_MODE(Iny) {
    address zeroL = READ(R_PC++);
    address zeroH = (zeroL + 1) & 0xFF;
    address addr = READ(zeroL) | (READ(zeroH) << 8);
    if (strcmp(instruction->operationName, "STA") != 0) {
        SPEND_IF_PAGE_CROSSED(addr, addr + R_Y, 1);
    }
    return addr + R_Y;
}

DEFINE_ADDRESS_MODE(Abi) {
    address abs = readAddress(R_PC);
    R_PC += 2;

    address effL = READ(abs);
#if 0
    address effH = READ(abs + 1);
#else
    address effH = READ((abs & 0xFF00) + ((abs + 1) & 0xFF));
#endif

    return effL + 0x100 * effH;
}

DEFINE_OPERATION(ADC) {
    byte data = READ(addr);
    uint16_t sum = R_A + data + F_C;
    FLAG_SET(CpuStatusFlag_Carry, sum & 0x100);
    FLAG_SET(CpuStatusFlag_Overflow, (R_A ^ sum) & (data ^ sum) & 0x80);
    R_A = (byte)sum;
    SET_ZN(R_A);
}

DEFINE_OPERATION(AND) {
    byte data = READ(addr);
    R_A &= data;
    SET_ZN(R_A);
}

DEFINE_OPERATION(ASL) {
    byte data = READ(addr);
    FLAG_SET(CpuStatusFlag_Carry, data & 0x80);
    data <<= 1;
    data &= 0xFF;
    SET_ZN(data);
    WRITE(addr, data);
}

DEFINE_OPERATION(ASL_acc) {
    byte data = R_A;
    FLAG_SET(CpuStatusFlag_Carry, data & 0x80);
    data <<= 1;
    data &= 0xFF;
    SET_ZN(data);
    R_A = data;
}

DEFINE_OPERATION(BCC) {
    if (!F_C) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BCS) {
    if (F_C) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BEQ) {
    if (F_Z) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BIT) {
    byte data = READ(addr);
    FLAG_SET(CpuStatusFlag_Zero, (R_A & data) == 0x00);
    FLAG_SET(CpuStatusFlag_Negative, data & 0x80);
    FLAG_SET(CpuStatusFlag_Overflow, data & 0x40);
}

DEFINE_OPERATION(BMI) {
    if (F_N) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BNE) {
    if (!F_Z) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BPL) {
    if (!F_N) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BRK) {
    R_PC++;
    pushStack((R_PC >> 8) & 0xFF);
    pushStack(R_PC & 0xFF);
    pushStack(R_P | CpuStatusFlag_Break);
    FLAG_SET(CpuStatusFlag_InterruptDisable, true);
    R_PC = readAddress(VECTOR_IRQ);
}

DEFINE_OPERATION(BVC) {
    if (!F_V) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(BVS) {
    if (F_V) {
        SPEND_IF_PAGE_CROSSED(R_PC, addr, 2);
        R_PC = addr;
        SPEND_CYCLES(1);
    }
}

DEFINE_OPERATION(CLC) {
    FLAG_SET(CpuStatusFlag_Carry, false);
}

DEFINE_OPERATION(CLD) {
    FLAG_SET(CpuStatusFlag_Decimal, false);
}

DEFINE_OPERATION(CLI) {
    FLAG_SET(CpuStatusFlag_InterruptDisable, false);
}

DEFINE_OPERATION(CLV) {
    FLAG_SET(CpuStatusFlag_Overflow, false);
}

DEFINE_OPERATION(CMP) {
    unsigned val = R_A - READ(addr);
    FLAG_SET(CpuStatusFlag_Carry, !(val & 0x100));
    SET_ZN(val & 0xFF);
}

DEFINE_OPERATION(CPX) {
    unsigned val = R_X - READ(addr);
    FLAG_SET(CpuStatusFlag_Carry, val < 0x100);
    SET_ZN(val);
}

DEFINE_OPERATION(CPY) {
    unsigned val = R_Y - READ(addr);
    FLAG_SET(CpuStatusFlag_Carry, val < 0x100);
    SET_ZN(val);
}

DEFINE_OPERATION(DEC) {
    byte value = READ(addr);
    value = (value - 1) & 0xFF;
    SET_ZN(value);
    WRITE(addr, value);
}

DEFINE_OPERATION(DEX) {
    R_X = (R_X - 1) & 0xFF;
    SET_ZN(R_X);
}

DEFINE_OPERATION(DEY) {
    R_Y = (R_Y - 1) & 0xFF;
    SET_ZN(R_Y);
}

DEFINE_OPERATION(EOR) {
    byte value = READ(addr);
    R_A ^= value;
    SET_ZN(R_A);
}

DEFINE_OPERATION(INC) {
    byte value = READ(addr);
    value = (value + 1) & 0xFF;
    SET_ZN(value);
    WRITE(addr, value);
}

DEFINE_OPERATION(INX) {
    R_X = (R_X + 1) & 0xFF;
    SET_ZN(R_X);
}

DEFINE_OPERATION(INY) {
    R_Y = (R_Y + 1) & 0xFF;
    SET_ZN(R_Y);
}

DEFINE_OPERATION(JMP) {
    R_PC = addr;
}

DEFINE_OPERATION(JSR) {
    R_PC--;
    pushStack((R_PC >> 8) & 0xFF);
    pushStack(R_PC & 0xFF);
    R_PC = addr;
}

DEFINE_OPERATION(LDA) {
    R_A = READ(addr);
    SET_ZN(R_A);
}

DEFINE_OPERATION(LDX) {
    R_X = READ(addr);
    SET_ZN(R_X);
}

DEFINE_OPERATION(LDY) {
    R_Y = READ(addr);
    SET_ZN(R_Y);
}

DEFINE_OPERATION(LSR) {
    byte value = READ(addr);
    FLAG_SET(CpuStatusFlag_Carry, value & 0x01);
    value >>= 1;
    FLAG_SET(CpuStatusFlag_Negative, false);
    FLAG_SET(CpuStatusFlag_Zero, value == 0x00);
    WRITE(addr, value);
}

DEFINE_OPERATION(LSR_acc) {
    byte value = R_A;
    FLAG_SET(CpuStatusFlag_Carry, value & 0x01);
    value >>= 1;
    FLAG_SET(CpuStatusFlag_Negative, false);
    FLAG_SET(CpuStatusFlag_Zero, value == 0x00);
    R_A = value;
}

DEFINE_OPERATION(NOP) { }

DEFINE_OPERATION(ORA) {
    byte value = READ(addr);
    R_A |= value;
    SET_ZN(R_A);
}

DEFINE_OPERATION(PHA) {
    pushStack(R_A);
}

DEFINE_OPERATION(PHP) {
    byte flags = R_P;
    flags |= CpuStatusFlag_Constant;
    flags |= CpuStatusFlag_Break;
    pushStack(flags);
}

DEFINE_OPERATION(PLA) {
    R_A = popStack();
    SET_ZN(R_A);
}

DEFINE_OPERATION(PLP) {
    byte flags = popStack();
    FLAG_SET(CpuStatusFlag_Negative, flags & 0x80);
    FLAG_SET(CpuStatusFlag_Overflow, flags & 0x40);
    FLAG_SET(CpuStatusFlag_Decimal, flags & 0x08);
    FLAG_SET(CpuStatusFlag_InterruptDisable, flags & 0x04);
    FLAG_SET(CpuStatusFlag_Zero, flags & 0x02);
    FLAG_SET(CpuStatusFlag_Carry, flags & 0x01);
}

DEFINE_OPERATION(ROL) {
    uint16_t value = READ(addr);
    value <<= 1;
    if (F_C) value |= 1;
    FLAG_SET(CpuStatusFlag_Carry, value > 0xFF);
    value &= 0xFF;
    SET_ZN(value);
    WRITE(addr, value);
}

DEFINE_OPERATION(ROL_acc) {
    uint16_t value = R_A;
    value <<= 1;
    if (F_C) value |= 1;
    FLAG_SET(CpuStatusFlag_Carry, value > 0xFF);
    value &= 0xFF;
    SET_ZN(value);
    R_A = value;
}

DEFINE_OPERATION(ROR) {
    uint16_t value = READ(addr);
    if (F_C) value |= 0x100;
    FLAG_SET(CpuStatusFlag_Carry, value & 0x01);
    value >>= 1;
    value &= 0xFF;
    SET_ZN(value);
    WRITE(addr, value);
}

DEFINE_OPERATION(ROR_acc) {
    uint16_t value = R_A;
    if (F_C) value |= 0x100;
    FLAG_SET(CpuStatusFlag_Carry, value & 0x01);
    value >>= 1;
    value &= 0xFF;
    SET_ZN(value);
    R_A = value;
}

DEFINE_OPERATION(RTI) {
    byte flags = popStack();
    FLAG_SET(CpuStatusFlag_Negative, flags & 0x80);
    FLAG_SET(CpuStatusFlag_Overflow, flags & 0x40);
    FLAG_SET(CpuStatusFlag_Decimal, flags & 0x08);
    FLAG_SET(CpuStatusFlag_InterruptDisable, flags & 0x04);
    FLAG_SET(CpuStatusFlag_Zero, flags & 0x02);
    FLAG_SET(CpuStatusFlag_Carry, flags & 0x01);
    R_PC = popStack() | (popStack() << 8);
}

DEFINE_OPERATION(RTS) {
    R_PC = (popStack() | (popStack() << 8)) + 1;
}

DEFINE_OPERATION(SBC) {
    byte value = READ(addr);
    unsigned tmp = R_A - value - (F_C ? 0 : 1);
    SET_ZN(tmp & 0xFF);
    FLAG_SET(CpuStatusFlag_Overflow, ((R_A ^ tmp) & 0x80) && ((R_A ^ value) & 0x80));
    FLAG_SET(CpuStatusFlag_Carry, tmp < 0x100);
    R_A = tmp & 0xFF;
}

DEFINE_OPERATION(SEC) {
    FLAG_SET(CpuStatusFlag_Carry, true);
}

DEFINE_OPERATION(SED) {
    FLAG_SET(CpuStatusFlag_Decimal, true);
}

DEFINE_OPERATION(SEI) {
    FLAG_SET(CpuStatusFlag_InterruptDisable, true);
}

DEFINE_OPERATION(STA) {
    WRITE(addr, R_A);
}

DEFINE_OPERATION(STX) {
    WRITE(addr, R_X);
}

DEFINE_OPERATION(STY) {
    WRITE(addr, R_Y);
}

DEFINE_OPERATION(TAX) {
    R_X = R_A;
    SET_ZN(R_X);
}

DEFINE_OPERATION(TAY) {
    R_Y = R_A;
    SET_ZN(R_Y);
}

DEFINE_OPERATION(TSX) {
    R_X = R_S;
    SET_ZN(R_X);
}

DEFINE_OPERATION(TXA) {
    R_A = R_X;
    SET_ZN(R_A);
}

DEFINE_OPERATION(TXS) {
    R_S = R_X;
}

DEFINE_OPERATION(TYA) {
    R_A = R_Y;
    SET_ZN(R_A);
}