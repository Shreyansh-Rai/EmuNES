#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Bus.h"
/*
What is it that a CPU needs ? 
- A bus to talk to other devices
- Needs to have some registers PC stkptr and stuff. Good, we have that.
  Need to decide the size of these and we have the data sheet for just this.
- opcodes to perform, well, ops -> how do WE do this? simple, look up the data sheet
    ask it what kind of ops does this cpu perform? What are the addressing modes that
    are used by it and how many cycles does this take? 
- Instructions are stored in the lookup_op vector.
TL;DR = regs, ops, addr modes for these ops for now these are the absolute basics.
*/
//Forward declaration since Bus needs access to the devices defined fully.
class Bus;

// Emulated Rockwell 6502. Many Thanks to OLC for teaching, and Rockwell for keeping a datasheet alive!
class R6502
{
public:
    R6502();
    ~R6502();

    void ConnectBus(Bus * b) { 
        bus = b;
    }

    // Processor Status Registers (PSRA and PSRB)
    // 7      6       5        4       3       2                       1       0
    // NEG(N) OVFL(V) NOT USED BRK (B) DEC (D) Interrupt ENBL/DISBL(I) ZERO(Z) CARRY(C)

    enum StatusRegisterFlags {
        C = (1<<0),
        Z = (1<<1),
        I = (1<<2),
        D = (1<<3),
        B = (1<<4),
        U = (1<<5),
        V = (1<<6),
        N = (1<<7)
    };

    //each CPU  has an 8-bit accu­mulator, two 8-bit index registers (X and Y), an 8-bit 
    //Stack Pointer Register,  an  8-bit  Status  Register,  a  16-bit  Program  Counter
    uint8_t accum = 0x00; //accumulator
    uint8_t xreg = 0x00; //x register
    uint8_t yreg = 0x00; //y register
    uint8_t stkptr = 0x00; //stack pointer -> will point to some location in the RAM.
    u_int16_t pc = 0x0000; //program counter
    uint8_t sreg = 0x00; //status register

    private: 
    //=========Addressing Modes and Opcodes===========================================
    //  Please Refer to rockwell_r65c00-21_r65c29.pdf 3-30 for the complete data sheet
    //======Addressing Modes =======
    // The 6502 addressing modes are methods the CPU uses to determine the location 
    // of the data (the operand) an instruction needs to operate on. The addressing 
    // mode dictates how many bytes the total instruction occupies, how many clock cycles
    // it will take to execute and so on. Primarily it seems there are 2 kinds of addressing
    // Direct : IMP - implied for example in INX -> increment X reg so the address is implied as xreg
    // IMM - Immediate or right after opcode you will get 1 Byte eg LDA #$10 
    // ABS - Absolute memory address is supplied eg, LDA $0800 this needs 3 bytes
    // ABX or say ZPX - These are indirect memory addressing. and require OP+2 bytes
    //                  AB is 0x0000-0xFFFF and ZP 0x0000-0x00FF range and X is the offset
    //                  from the AB or ZP value think of it like an array AB[X]/ZP[X]
    uint8_t IMP();	
    uint8_t IMM();	
	uint8_t ZP0();	
    uint8_t ZPX();	
	uint8_t ZPY();	
    uint8_t REL();
	uint8_t ABS();	
    uint8_t ABX();	
	uint8_t ABY();	
    uint8_t IND();	
	uint8_t IZX();	
    uint8_t IZY();
    //======OP Codes================
    //Since we have an entire byte to represent it, there could be 256 but there are only 56 here
    //Described in the data sheet. As suggested in the REPO of OLC, mapping the rest to XXX a dummy code.
    //Why the uint8_t return you would wonder? Another goated suggestion by OLC, and discord for emu
    //Essentially take the case of EOR in ZP it takes 3 cycles (Check data sheet) and in ZPX 4.
    //Hence we can return values to essentially wait for extra ticks of the clock. Since
    //all the ops we do are kind of done in O(1) and any extra time is spent waiting for clock cycles
    //since it is EXPECTED of the CPU.
    uint8_t ADC();	uint8_t AND();	uint8_t ASL();	uint8_t BCC();
	uint8_t BCS();	uint8_t BEQ();	uint8_t BIT();	uint8_t BMI();
	uint8_t BNE();	uint8_t BPL();	uint8_t BRK();	uint8_t BVC();
	uint8_t BVS();	uint8_t CLC();	uint8_t CLD();	uint8_t CLI();
	uint8_t CLV();	uint8_t CMP();	uint8_t CPX();	uint8_t CPY();
	uint8_t DEC();	uint8_t DEX();	uint8_t DEY();	uint8_t EOR();
	uint8_t INC();	uint8_t INX();	uint8_t INY();	uint8_t JMP();
	uint8_t JSR();	uint8_t LDA();	uint8_t LDX();	uint8_t LDY();
	uint8_t LSR();	uint8_t NOP();	uint8_t ORA();	uint8_t PHA();
	uint8_t PHP();	uint8_t PLA();	uint8_t PLP();	uint8_t ROL();
	uint8_t ROR();	uint8_t RTI();	uint8_t RTS();	uint8_t SBC();
	uint8_t SEC();	uint8_t SED();	uint8_t SEI();	uint8_t STA();
	uint8_t STX();	uint8_t STY();	uint8_t TAX();	uint8_t TAY();
	uint8_t TSX();	uint8_t TXA();	uint8_t TXS();	uint8_t TYA();

    //Now we have a NOP
    uint8_t XXX();

    //Interrupts and CLOCK
    // External event functions. In hardware these represent pins that are asserted
	// to produce a change in state.
    // In each case, the interrupt condition is reported as an interrupt 
    // flag  in  a  control/status  register  associated  with  the  functional 
    // area. Each CPU can either enable or disable IRQ generation by 
    // setting or resetting a corresponding  interrupt enable bit  in the 
    // same or associated controVstatus register.
    // Furthermore,  each  CPU  can  control  whether  or_not  its  pro­
    // cessing is interrupted when an  interrupt request (IRQ) is gen­
    // erated. Each CPU has its own Processor Status Register (PSRA 
    // and PSRB) with the capability of disabling IRQ interrupts when 
    // its own “I flag" bit is a 1.
    // 3-10
	void reset();	// Reset Interrupt - Forces CPU into known state
	void irq();		// Interrupt Request - Executes an instruction at a specific location
	void nmi();		// Non-Maskable Interrupt Request - As above, but cannot be disabled
	void clock();	// Perform one clock cycle's worth of update

    //Some Helpers to cleanly fetch data
    uint8_t fetch();
    uint8_t fetched = 0x00; //data that was fetched.
    uint16_t addr_loc = 0x0000;
    uint16_t addr_off = 0x0000; 
    uint8_t cur_op = 0x00; //opcode for current instruction
    uint8_t cur_cycles = 0; //cycles to go for current opcode.

    uint8_t GetFlag(StatusRegisterFlags f);
	void    SetFlag(StatusRegisterFlags f, bool v);

    //Instruction to work on - 
    struct op {
        std::string name;
        uint8_t(R6502::*opfunction)(void) = nullptr; //opcode function ptr
        uint8_t(R6502::*addrmode)(void) = nullptr; // addressmode function ptr
        uint8_t cycles_req = 0; //stores the cycles needed for the op 
    };

    std::vector<op> op_lookup;

private : 
    Bus * bus = nullptr;
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
};