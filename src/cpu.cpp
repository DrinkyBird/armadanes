#include <cstdio>
#include <cstring>
#include "cpu.h"
#include "system.h"
#include "cpubus.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')


Cpu::Cpu(System *system) {
    this->system = system;
    this->cyclesToSkip = 0;
    this->totalCycles = 7;
    setupInstructions();
    log = fopen("cpu.log", "w");
}

Cpu::~Cpu() {

}

void Cpu::start() {
    registers.p = 0x24; // should be 0x34, but nestest.log uses 0x24, and the affected bits don't matter anyway
    registers.a = 0;
    registers.x = 0;
    registers.y = 0;
    registers.pc = readAddress(VECTOR_RESET);
    registers.pc = 0xC000; // for nestest
    registers.s = 0xFD;
}

void Cpu::reset() {
    registers.s -= 3;
    registers.pc = readAddress(VECTOR_RESET);
    registers.p |= 0x04;
}

void Cpu::step() {
    char logline[512];

    // run a cycle
    if (cyclesToSkip > 0) {
        cyclesToSkip--;
        totalCycles++;
        return;
    }

    address pc = registers.pc;
    byte opcode = system->getBus()->read(registers.pc++);
    CpuInstruction *instruction = &instructions[opcode];

    if (!instruction->legal) {
        snprintf(logline, sizeof(logline), "%04X Illegal instruction %02X\n", pc, opcode);
        fwrite(logline, sizeof(char), strlen(logline), log); fflush(log);
        printf("Illegal instruction %02x\n", opcode);
    }

    if (instruction->addressingMode != nullptr && instruction->operation != nullptr) {
        address addr = (this->*instruction->addressingMode)(instruction);

        snprintf(logline, sizeof(logline), "%04X %02X (%s %s with %04X = %02X) cycle: %u, A: %02X, X: %02X, Y: %02X, S: %02X, P: " BYTE_TO_BINARY_PATTERN " %02X\n",
                 pc, opcode, instruction->operationName, instruction->addressingModeName, addr, system->getBus()->read(addr), totalCycles, registers.a, registers.x, registers.y, registers.s, BYTE_TO_BINARY(registers.p), registers.p);
        fwrite(logline, sizeof(char), strlen(logline), log); fflush(log);

        (this->*instruction->operation)(instruction, addr);
        cyclesToSkip += instruction->cycles - 1;
    }

    totalCycles++;
}

void Cpu::pushStack(byte val) {
    system->getBus()->write(0x0100 + registers.s, val);
    if (registers.s == 0x00) registers.s = 0xFF;
    else registers.s--;
}

byte Cpu::popStack() {
    if (registers.s == 0xFF) registers.s = 0x00;
    else registers.s++;
    return system->getBus()->read(0x0100 + registers.s);
}

address Cpu::readAddress(address ptr) {
    return system->getBus()->read(ptr) | (system->getBus()->read(ptr + 1) << 8);
}

void Cpu::generateIrq() {
    if (!(registers.p & CpuStatusFlag_InterruptDisable)) {
        if (registers.p & CpuStatusFlag_Break) {
            registers.p &= ~CpuStatusFlag_Break;
        }

        pushStack((registers.pc >> 8) & 0xFF);
        pushStack(registers.pc & 0xFF);
        pushStack(registers.p);
        registers.p |= CpuStatusFlag_InterruptDisable;
        registers.pc = readAddress(VECTOR_IRQ);
    }
}

void Cpu::generateNmi() {
    if (registers.p & CpuStatusFlag_Break) {
        registers.p &= ~CpuStatusFlag_Break;
    }

    pushStack((registers.pc >> 8) & 0xFF);
    pushStack(registers.pc & 0xFF);
    pushStack(registers.p);
    registers.p |= CpuStatusFlag_InterruptDisable;
    registers.pc = readAddress(VECTOR_NMI);
}