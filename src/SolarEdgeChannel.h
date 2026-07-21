#pragma once
#include "BaseMeterChannel.h"

// SolarEdge SunSpec (Erzeuger / Wechselrichter). Emuliert einen einphasigen
// SolarEdge-Wechselrichter (DID 101). FC3. Der EX.1 liest den Ident-Block ab
// 40000 und Live-Werte an rohen Adressen (Leistung@83, Status@107) -> dieselbe
// 123-Word-Registerkarte wird doppelt (roh + 40000) gemappt.
// Referenz: Sunspec bridge sunspec_server.py build_registers().
class SolarEdgeChannel : public BaseMeterChannel
{
    static const uint16_t REG_COUNT = 123;

    char _model[40] = "SE3000H-RW000BNN4";
    char _serial[40] = "7E123456";

    // Lebens-Energiezähler (Wh), integriert aus der Leistung, flash-persistiert.
    double _whTotal = 0.0;
    uint32_t _lastIntegrateMs = 0;

    static void strToRegs(const char* s, uint16_t* out, uint16_t count);

  public:
    explicit SolarEdgeChannel(uint8_t channelIndex) : BaseMeterChannel(channelIndex) {}

    uint8_t functionCode() const override { return EEX_FC_READ_HOLDING; }
    bool isProducer() const override { return true; }

    void setup() override;
    void loop() override;
    void buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out) override;

    bool hasFlashState() const override { return true; }
    void saveFlashState(uint8_t* buffer) const override;
    void loadFlashState(const uint8_t* buffer) override;
};
