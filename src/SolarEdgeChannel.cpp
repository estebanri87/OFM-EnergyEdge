#include "SolarEdgeChannel.h"
#include "knxprod.h"
#include <string.h>
#include <math.h>

#define SUNSPEC_IDENT_BASE 40000

static inline uint16_t s16(int v) { return (uint16_t)(v & 0xFFFF); }

void SolarEdgeChannel::strToRegs(const char* s, uint16_t* out, uint16_t count)
{
    for (uint16_t i = 0; i < count; i++)
        out[i] = 0;
    for (uint16_t i = 0; s[i] != '\0' && i < (uint16_t)(count * 2); i++)
    {
        uint16_t idx = i / 2;
        if ((i & 1) == 0)
            out[idx] |= (uint16_t)((uint8_t)s[i] << 8);
        else
            out[idx] |= (uint16_t)((uint8_t)s[i]);
    }
}

void SolarEdgeChannel::setup()
{
    BaseMeterChannel::setup();
    // ETS-Textfelder sind 32 Byte lang (PT-EEXString32); nur so viel lesen.
    const char* model = (const char*)ParamEEX_CHModel;
    const char* serial = (const char*)ParamEEX_CHSerial;
    if (model && model[0])
    {
        strncpy(_model, model, 32);
        _model[32] = '\0';
    }
    if (serial && serial[0])
    {
        strncpy(_serial, serial, 32);
        _serial[32] = '\0';
    }
    _lastIntegrateMs = millis();
}

void SolarEdgeChannel::loop()
{
    // Energieintegration (Wh) aus der Leistung, wie sunspec_server.py:
    // wh_total += watt * dt / 3600 (dt in Sekunden). Nur bei positiver Leistung.
    uint32_t now = millis();
    float watt = currentWatt();
    if (watt > 0.0f)
    {
        float dtSec = (now - _lastIntegrateMs) / 1000.0f;
        _whTotal += (double)watt * dtSec / 3600.0;
    }
    _lastIntegrateMs = now;
}

void SolarEdgeChannel::buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out)
{
    memset(out, 0, (size_t)words * sizeof(uint16_t));

    uint16_t r[REG_COUNT];
    for (uint16_t i = 0; i < REG_COUNT; i++)
        r[i] = 0;

    float watt = currentWatt();

    // SunSpec-Header / Common Model
    r[0] = 0x5375; r[1] = 0x6E53; // "SunS"
    r[2] = 1; r[3] = 65;
    uint16_t tmp[16];
    strToRegs("SolarEdge", tmp, 16); for (int i = 0; i < 16; i++) r[4 + i] = tmp[i];
    strToRegs(_model, tmp, 16);      for (int i = 0; i < 16; i++) r[20 + i] = tmp[i];
    strToRegs("0004", tmp, 8);       for (int i = 0; i < 8; i++)  r[44 + i] = tmp[i];
    strToRegs(_serial, tmp, 16);     for (int i = 0; i < 16; i++) r[52 + i] = tmp[i];
    r[68] = 1;
    r[69] = 101; r[70] = 50; // Inverter DID 101 (einphasig) / Länge 50

    int iL1 = (int)lroundf(watt / 230.0f);
    r[71] = s16(iL1); r[72] = s16(iL1);
    r[73] = 0; r[74] = 0; r[75] = 0;
    r[76] = 2300; r[77] = 0; r[78] = 0;
    r[79] = 2300; r[80] = 0; r[81] = 0; r[82] = s16(-1);

    uint16_t powerReg = s16((int)lroundf(watt / 10.0f)); // SF=1 -> Wert*10 = W
    r[83] = powerReg; r[84] = s16(1);   // AC Power + Scale Factor
    r[85] = 5000; r[86] = s16(-2);      // 50,00 Hz
    r[87] = 0; r[88] = 0; r[89] = 0; r[90] = 0;
    r[91] = 0; r[92] = 0;

    uint32_t whI = (uint32_t)llround(_whTotal);
    r[93] = (uint16_t)((whI >> 16) & 0xFFFF);
    r[94] = (uint16_t)(whI & 0xFFFF);
    r[95] = 0;
    r[96] = s16(iL1); r[97] = 0;
    r[98] = 600; r[99] = 0;
    r[100] = powerReg; r[101] = s16(1);
    r[103] = s16(350); r[106] = s16(-1); // 35,0 °C
    r[107] = (watt > 0.0f) ? 4 : 2;      // I_Status: 4=Producing, 2=Sleeping
    r[108] = 0;                          // Vendor-Status: kein Fehler
    r[121] = 0xFFFF;                     // End-Marker

    // Doppelte Ablage: rohe Basis (Live 83/107) und Ident-Block ab 40000.
    for (uint16_t i = 0; i < words; i++)
    {
        uint32_t a = (uint32_t)reqAddr + i;
        if (a < REG_COUNT)
            out[i] = r[a];
        else if (a >= SUNSPEC_IDENT_BASE && (a - SUNSPEC_IDENT_BASE) < REG_COUNT)
            out[i] = r[a - SUNSPEC_IDENT_BASE];
    }
}

void SolarEdgeChannel::saveFlashState(uint8_t* buffer) const
{
    memcpy(buffer, &_whTotal, sizeof(_whTotal)); // 8 bytes = EEX_FLASH_SLOT_SIZE
}

void SolarEdgeChannel::loadFlashState(const uint8_t* buffer)
{
    memcpy(&_whTotal, buffer, sizeof(_whTotal));
    if (!(_whTotal >= 0.0))
        _whTotal = 0.0;
}
