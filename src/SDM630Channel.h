#pragma once
#include "BaseMeterChannel.h"

// Eastron SDM630 (Verbraucher, alternativ). FC4 (Input Register), Wirkleistung
// als IEEE-754 Float (BE, High-Word zuerst) ab Register 52, Einheit W (keine
// kW-Skalierung). Unit-ID fest 1. Referenz: EX1-Modbus-Spec.md §4.
class SDM630Channel : public BaseMeterChannel
{
  public:
    explicit SDM630Channel(uint8_t channelIndex) : BaseMeterChannel(channelIndex) {}

    uint8_t functionCode() const override { return EEX_FC_READ_INPUT; }
    void buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out) override;
};
