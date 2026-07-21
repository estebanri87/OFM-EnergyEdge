#include "KElectricChannel.h"
#include <string.h>

#define KELECTRIC_POWER_REG 4151

bool KElectricChannel::s_energyProbe = false;

void KElectricChannel::buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out)
{
    memset(out, 0, (size_t)words * sizeof(uint16_t));

    // KElectric erwartet kW -> Watt / 1000.
    float kW = currentWatt() / 1000.0f;
    uint16_t hi, lo;
    eexFloatToRegsBE(kW, hi, lo);
    putReg(reqAddr, words, out, KELECTRIC_POWER_REG, hi);     // 4151 High-Word
    putReg(reqAddr, words, out, KELECTRIC_POWER_REG + 1, lo); // 4152 Low-Word

    if (s_energyProbe)
    {
        // Sentinel: jedes Energie-Float-Slot der beiden vom EX.1 gelesenen Blöcke
        // (4131..4150, 4173..4192) = seine Registeradresse als Float. Der EX.1-Wert
        // fuer Bezug/Einspeisung zeigt dann direkt das zugehörige Register.
        for (uint16_t reg = 4131; reg <= 4149; reg += 2)
        {
            eexFloatToRegsBE((float)reg, hi, lo);
            putReg(reqAddr, words, out, reg, hi);
            putReg(reqAddr, words, out, reg + 1, lo);
        }
        for (uint16_t reg = 4173; reg <= 4191; reg += 2)
        {
            eexFloatToRegsBE((float)reg, hi, lo);
            putReg(reqAddr, words, out, reg, hi);
            putReg(reqAddr, words, out, reg + 1, lo);
        }
    }
}
