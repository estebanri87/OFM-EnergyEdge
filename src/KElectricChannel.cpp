#include "KElectricChannel.h"
#include <string.h>

#define KELECTRIC_POWER_REG  4151
// Energieregister, am Gerät per eexprobe ermittelt: der EX.1 liest die Bloecke
// 4131..4150 und 4173..4192 und summiert je Wert Slot 0 und Slot 8 (Zweitarifzaehler).
#define KELECTRIC_IMPORT_REG 4131 // zweiter Summand liegt auf 4147, bleibt 0
#define KELECTRIC_EXPORT_REG 4173 // zweiter Summand liegt auf 4189, bleibt 0

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

    // Smart Meter: Bezug/Einspeisung in kWh. Der EX.1 summiert je Wert zwei Register
    // (Bezug 4131+4147, Einspeisung 4173+4189 - per eexprobe am Gerät ermittelt). Wir
    // legen den vollen Wert auf das jeweils erste; das zweite bleibt durch das memset 0,
    // die Summe ergibt also exakt den KNX-Wert.
    if (!s_energyProbe && isSmartMeter())
    {
        eexFloatToRegsBE(importKwh(), hi, lo);
        putReg(reqAddr, words, out, KELECTRIC_IMPORT_REG, hi);
        putReg(reqAddr, words, out, KELECTRIC_IMPORT_REG + 1, lo);

        eexFloatToRegsBE(exportKwh(), hi, lo);
        putReg(reqAddr, words, out, KELECTRIC_EXPORT_REG, hi);
        putReg(reqAddr, words, out, KELECTRIC_EXPORT_REG + 1, lo);
    }

    if (s_energyProbe)
    {
        // Sentinel mit Binaergewichten: Slot n der beiden vom EX.1 gelesenen Energie-
        // bloecke (4131..4150, 4173..4192) bekommt 10 * 2^n kWh, wird im EX.1 also als
        // 0,01 * 2^n MWh angezeigt. Der angezeigte MWh-Wert * 100 ist damit die Binaer-
        // summe der tatsaechlich genutzten Slots und eindeutig dekodierbar - auch wenn
        // der EX.1 mehrere Register addiert (Adressen als Marker waeren symmetrisch und
        // damit mehrdeutig). Maximalwert 10,23 MWh haelt die EX.1-Statistik sauber.
        uint8_t slot = 0;
        for (uint16_t reg = 4131; reg <= 4149; reg += 2, slot++)
        {
            eexFloatToRegsBE(10.0f * (float)(1UL << slot), hi, lo);
            putReg(reqAddr, words, out, reg, hi);
            putReg(reqAddr, words, out, reg + 1, lo);
        }
        slot = 0;
        for (uint16_t reg = 4173; reg <= 4191; reg += 2, slot++)
        {
            eexFloatToRegsBE(10.0f * (float)(1UL << slot), hi, lo);
            putReg(reqAddr, words, out, reg, hi);
            putReg(reqAddr, words, out, reg + 1, lo);
        }
    }
}
