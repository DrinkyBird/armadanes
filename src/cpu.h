#pragma once
#include <cstdio>
#include "armadadef.h"
#include "cpudefs.h"

class System;
class CpuBus;

class Cpu {
public:
    Cpu(System *system);
    ~Cpu();

    void start();
    void reset();
    void step();

    void pushStack(byte val);
    byte popStack();

    address readAddress(address ptr);

    void generateIrq();
    void generateNmi();

    CpuRegisters registers;

private:
    void setupInstructions();

    System *system;
    CpuInstruction instructions[0xFF];
    unsigned cyclesToSkip;
    unsigned totalCycles;

    FILE *log;

#define DECLARE_ADDRESS_MODE(mnemonic) \
    address addr##mnemonic(CpuInstruction *)

    DECLARE_ADDRESS_MODE(Acc); // accumulator
    DECLARE_ADDRESS_MODE(Imm); // immediate
    DECLARE_ADDRESS_MODE(Abs); // absolute
    DECLARE_ADDRESS_MODE(Abx); // indexed-x absolute
    DECLARE_ADDRESS_MODE(Aby); // indexed-y absolute
    DECLARE_ADDRESS_MODE(Zer); // zero page
    DECLARE_ADDRESS_MODE(Zex); // indexed-x zero page
    DECLARE_ADDRESS_MODE(Zey); // indexed-x zero page
    DECLARE_ADDRESS_MODE(Imp); // implied
    DECLARE_ADDRESS_MODE(Rel); // relative
    DECLARE_ADDRESS_MODE(Inx); // indexed-y indirect
    DECLARE_ADDRESS_MODE(Iny); // indexed-y indirect
    DECLARE_ADDRESS_MODE(Abi); // absolute indirect

#undef DECLARE_ADDRESS_MODE
#define DECLARE_OPERATION(mnemonic) \
    void op##mnemonic(CpuInstruction *, address)

    DECLARE_OPERATION(ADC);
    DECLARE_OPERATION(AND);
    DECLARE_OPERATION(ASL);
    DECLARE_OPERATION(ASL_acc);
    DECLARE_OPERATION(BCC);
    DECLARE_OPERATION(BCS);
    DECLARE_OPERATION(BEQ);
    DECLARE_OPERATION(BIT);
    DECLARE_OPERATION(BMI);
    DECLARE_OPERATION(BNE);
    DECLARE_OPERATION(BPL);
    DECLARE_OPERATION(BRK);
    DECLARE_OPERATION(BVC);
    DECLARE_OPERATION(BVS);
    DECLARE_OPERATION(CLC);
    DECLARE_OPERATION(CLD);
    DECLARE_OPERATION(CLI);
    DECLARE_OPERATION(CLV);
    DECLARE_OPERATION(CMP);
    DECLARE_OPERATION(CPX);
    DECLARE_OPERATION(CPY);
    DECLARE_OPERATION(DEC);
    DECLARE_OPERATION(DEX);
    DECLARE_OPERATION(DEY);
    DECLARE_OPERATION(EOR);
    DECLARE_OPERATION(INC);
    DECLARE_OPERATION(INX);
    DECLARE_OPERATION(INY);
    DECLARE_OPERATION(JMP);
    DECLARE_OPERATION(JSR);
    DECLARE_OPERATION(LDA);
    DECLARE_OPERATION(LDX);
    DECLARE_OPERATION(LDY);
    DECLARE_OPERATION(LSR);
    DECLARE_OPERATION(LSR_acc);
    DECLARE_OPERATION(NOP);
    DECLARE_OPERATION(ORA);
    DECLARE_OPERATION(PHA);
    DECLARE_OPERATION(PHP);
    DECLARE_OPERATION(PLA);
    DECLARE_OPERATION(PLP);
    DECLARE_OPERATION(ROL);
    DECLARE_OPERATION(ROL_acc);
    DECLARE_OPERATION(ROR);
    DECLARE_OPERATION(ROR_acc);
    DECLARE_OPERATION(RTI);
    DECLARE_OPERATION(RTS);
    DECLARE_OPERATION(SBC);
    DECLARE_OPERATION(SEC);
    DECLARE_OPERATION(SED);
    DECLARE_OPERATION(SEI);
    DECLARE_OPERATION(STA);
    DECLARE_OPERATION(STX);
    DECLARE_OPERATION(STY);
    DECLARE_OPERATION(TAX);
    DECLARE_OPERATION(TAY);
    DECLARE_OPERATION(TSX);
    DECLARE_OPERATION(TXA);
    DECLARE_OPERATION(TXS);
    DECLARE_OPERATION(TYA);

#undef DECLARE_OPERATION
};


