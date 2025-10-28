#pragma once
#include <cstdint>
#include "R6502.h"
#include <array>
//Bus interface for devices to communicate w/ CPU
class Bus
{
public:
    Bus();
    ~Bus();

    //Connected devices 
    R6502 cpu;
    std::array<uint8_t, 64*1024> ram; //0x0000->0xFFFF or 2^16 bits of address space available to us.
    
    //We have a 16 bit PC to work with incase of 6502.
    void write(uint16_t addr, uint8_t data);
    uint8_t read(uint16_t addr, bool bReadOnly = false);
};