#pragma once
#include "OpenKNX.h"

// Category values (ETS PT-EEXCategory enumeration). Primary, user-facing.
#define EEX_CAT_ENERGYMETER 0 // Energiemessung / SmartMeter
#define EEX_CAT_SMARTPLUG   1 // SmartPlug
#define EEX_CAT_APPLIANCE   2 // Haushaltsgerät
#define EEX_CAT_SWITCH      3 // Schalter
#define EEX_CAT_INVERTER    4 // Wechselrichter

// Consumer profile values (ETS PT-EEXConsumerProfile). Inverter category implies
// SolarEdge SunSpec; consumer categories choose between these two.
#define EEX_CONSUMER_KELECTRIC 0 // KElectric KE-P-80: FC3, reg 4151, kW, unit-id per device
#define EEX_CONSUMER_SDM630    1 // Eastron SDM630: FC4, reg 52, W, unit-id fixed 1

// Modbus function codes
#define EEX_FC_READ_HOLDING 3
#define EEX_FC_READ_INPUT   4

// Build big-endian, high-word-first IEEE-754 float registers, matching the
// reference add-ons' float_to_regs(struct.pack(">f", value)).
void eexFloatToRegsBE(float value, uint16_t& highWord, uint16_t& lowWord);

class BaseMeterChannel : public OpenKNX::Channel
{
  protected:
    uint8_t _channelIndex;
    uint8_t _category = EEX_CAT_ENERGYMETER;
    uint8_t _unitId = 1;
    uint16_t _port = 502;
    uint32_t _watchdogMs = 0; // 0 = disabled
    bool _invert = false;
    float _scale = 1.0f;

    // Latest KNX power value (W). Written by the KNX loop in processInputKo(),
    // read by the Modbus worker (AsyncTCP task). 32-bit aligned -> atomic on ESP32.
    volatile float _watt = 0.0f;
    volatile uint32_t _lastUpdateMs = 0;

    // Helper for concrete profiles: set out[addr-base] = value if base is inside
    // the requested window [reqAddr, reqAddr+words).
    static void putReg(uint16_t reqAddr, uint16_t words, uint16_t* out, uint16_t regAddr, uint16_t value);

  public:
    explicit BaseMeterChannel(uint8_t channelIndex);
    virtual ~BaseMeterChannel() = default;

    const std::string name() override;
    void setup() override;
    void loop() override;
    void processInputKo(GroupObject& ko) override;

    // --- Modbus interface (read by EnergyEdgeServer worker) ---
    uint8_t unitId() const { return _unitId; }
    uint16_t port() const { return _port; }
    virtual uint8_t functionCode() const = 0; // EEX_FC_READ_HOLDING / _READ_INPUT

    // Producer profiles (inverter) use the producer port; consumers the consumer port.
    virtual bool isProducer() const { return false; }

    // Fill out[0..words-1] with register values for addresses reqAddr..reqAddr+words-1
    // (0 where undefined -> avoids IllegalAddress). Called from the AsyncTCP task.
    virtual void buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out) = 0;

    // Current power in W after watchdog + invert + scale.
    float currentWatt();

    // --- optional persisted per-channel state (SolarEdge energy counter) ---
    virtual bool hasFlashState() const { return false; }
    virtual void saveFlashState(uint8_t* buffer) const {}         // buffer >= EEX_FLASH_SLOT_SIZE
    virtual void loadFlashState(const uint8_t* buffer) {}
};

// Fixed per-channel flash slot (double Wh for SolarEdge). Keep stable across versions.
#define EEX_FLASH_SLOT_SIZE 8
