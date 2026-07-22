#pragma once
#include "OpenKNX.h"
#include "ModbusSource.h"

// Category values (ETS PT-EEXCategory enumeration). Primary, user-facing.
#define EEX_CAT_ENERGYMETER 0 // Energiemessung (nur Wirkleistung)
#define EEX_CAT_SMARTMETER  1 // Smart Meter (zusätzlich Bezug/Einspeisung)
#define EEX_CAT_SWITCH      2 // Schalter/Virtual Switch (EX.1 -> KNX)
#define EEX_CAT_INVERTER    3 // Wechselrichter

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

class BaseMeterChannel : public OpenKNX::Channel, public ModbusSource
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

    // Smart-Meter-Energie (kWh, DPT 13.013). Kein Watchdog: ein Zaehlerstand darf nicht
    // auf 0 fallen, nur weil gerade kein Telegramm kam.
    volatile float _importKwh = 0.0f;
    volatile float _exportKwh = 0.0f;

    // Modbus request tracking (status KOs). Written by the AsyncTCP worker
    // (onModbusRequest), read/consumed by the module loop.
    volatile uint32_t _lastRequestMs = 0;
    volatile bool _requestSeen = false;

    // Diagnose: bis zu 8 distinkte (Startadresse, Wortanzahl), die der EX.1 an dieser
    // Unit-ID liest (Registermitschnitt, z.B. für die Smart-Meter-Energieregister).
    struct ReqRec
    {
        uint16_t addr;
        uint16_t words;
    };
    ReqRec _reqLog[8];
    volatile uint8_t _reqLogCount = 0;

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

    // Smart-Meter-Energie in kWh (1:1 durchgereicht, kein Watchdog).
    float importKwh() const { return _importKwh; }
    float exportKwh() const { return _exportKwh; }
    bool isSmartMeter() const { return _category == EEX_CAT_SMARTMETER; }

    // --- Modbus request tracking (status KOs) + Registermitschnitt ---
    void onModbusRequest(uint16_t addr, uint16_t words) override
    {
        _lastRequestMs = millis();
        _requestSeen = true;
        for (uint8_t i = 0; i < _reqLogCount; i++)
            if (_reqLog[i].addr == addr && _reqLog[i].words == words)
                return; // schon erfasst
        if (_reqLogCount < 8)
        {
            _reqLog[_reqLogCount].addr = addr;
            _reqLog[_reqLogCount].words = words;
            _reqLogCount = (uint8_t)(_reqLogCount + 1); // kein ++ auf volatile (C++20)
        }
    }
    uint32_t lastRequestMs() const { return _lastRequestMs; }
    bool consumeRequestSeen()
    {
        bool s = _requestSeen;
        _requestSeen = false;
        return s;
    }
    uint8_t reqLogCount() const { return _reqLogCount; }
    uint16_t reqLogAddr(uint8_t i) const { return _reqLog[i].addr; }
    uint16_t reqLogWords(uint8_t i) const { return _reqLog[i].words; }

    // --- optional persisted per-channel state (SolarEdge energy counter) ---
    virtual bool hasFlashState() const { return false; }
    virtual void saveFlashState(uint8_t* buffer) const {}         // buffer >= EEX_FLASH_SLOT_SIZE
    virtual void loadFlashState(const uint8_t* buffer) {}
};

// Fixed per-channel flash slot (double Wh for SolarEdge). Keep stable across versions.
#define EEX_FLASH_SLOT_SIZE 8
