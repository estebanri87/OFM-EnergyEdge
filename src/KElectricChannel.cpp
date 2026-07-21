#include "KElectricChannel.h"
#include <string.h>

#define KELECTRIC_POWER_REG 4151

void KElectricChannel::buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out)
{
    memset(out, 0, (size_t)words * sizeof(uint16_t));

    // KElectric erwartet kW -> Watt / 1000.
    float kW = currentWatt() / 1000.0f;
    uint16_t hi, lo;
    eexFloatToRegsBE(kW, hi, lo);
    putReg(reqAddr, words, out, KELECTRIC_POWER_REG, hi);     // 4151 High-Word
    putReg(reqAddr, words, out, KELECTRIC_POWER_REG + 1, lo); // 4152 Low-Word
}
