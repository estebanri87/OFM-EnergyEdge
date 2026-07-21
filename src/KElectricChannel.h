#pragma once
#include "BaseMeterChannel.h"

// KElectric KE-P-80 (Verbraucher). FC3, Wirkleistung als IEEE-754 Float (BE,
// High-Word zuerst) ab Holding-Register 4151, Einheit kW. Unit-ID pro Gerät.
// Referenz: KNX Meter bridge meter_server.py.
class KElectricChannel : public BaseMeterChannel
{
  public:
    explicit KElectricChannel(uint8_t channelIndex) : BaseMeterChannel(channelIndex) {}

    uint8_t functionCode() const override { return EEX_FC_READ_HOLDING; }
    void buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out) override;

    // Diagnose (Konsole "eexprobe"): belegt die vom EX.1 gelesenen Energieblöcke
    // 4131..4150 und 4173..4192 mit Sentinel-Werten (= Registeradresse als Float),
    // damit der im EX.1 angezeigte Bezug/Einspeisung-Wert das Register verrät.
    static void setEnergyProbe(bool on) { s_energyProbe = on; }
    static bool energyProbe() { return s_energyProbe; }

  private:
    static bool s_energyProbe;
};
