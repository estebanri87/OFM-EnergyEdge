#include "SDM630Channel.h"
#include <string.h>

#define SDM630_POWER_REG 52

void SDM630Channel::buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out)
{
    memset(out, 0, (size_t)words * sizeof(uint16_t));

    // SDM630 erwartet Watt direkt (keine kW-Skalierung).
    float watt = currentWatt();
    uint16_t hi, lo;
    eexFloatToRegsBE(watt, hi, lo);
    putReg(reqAddr, words, out, SDM630_POWER_REG, hi);     // 52 High-Word
    putReg(reqAddr, words, out, SDM630_POWER_REG + 1, lo); // 53 Low-Word
}
