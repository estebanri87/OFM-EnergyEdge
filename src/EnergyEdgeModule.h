#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "EnergyEdgeServer.h"
#include <vector>

class BaseMeterChannel;
class VirtualSwitchChannel;

// KNX <-> ABB Energy Edge EX.1 bridge. Metering direction (KNX -> Modbus) via emulated
// Modbus meters (KElectric / SDM630 / SolarEdge); switching direction (EX.1 -> KNX) via
// the Virtual-Switch channel (poll /v2/point -> KNX actuator).
class EnergyEdgeModule : public EEXChannelOwnerModule
{
    EnergyEdgeServer _server;
    std::vector<BaseMeterChannel*> _meters;
    std::vector<VirtualSwitchChannel*> _switches;

    // Category-safe accessors (getChannel() returns the polymorphic base; a Schalter
    // channel is a VirtualSwitchChannel, not a BaseMeterChannel).
    BaseMeterChannel* meterAt(uint8_t channelIndex);
    VirtualSwitchChannel* switchAt(uint8_t channelIndex);

    // EX.1-API-Poll (EX.1 -> KNX): GET /v2/point, treibt Virtual-Switch-Ausgänge und die
    // EX.1-Messwerte. Ein Request je Intervall für Schalter + Messwerte.
    uint32_t _lastSwitchPollMs = 0;
    void pollEx1Api();
    void processPointResponse(const std::string& body);

    // EX.1 Messwerte (API -> KNX): send-on-change (Hysterese) + zyklisch, je Wert.
    struct MvState
    {
        float lastSent = 0.0f;
        uint32_t lastSend = 0;
        bool sent = false;
    };
    MvState _mvPv, _mvCons, _mvBat, _mvSoc;
    bool measurementsEnabled();
    void sendMeasurement(MvState& st, bool enabled, uint8_t changeMode, float threshold,
                         uint32_t cyclicMs, float value, GroupObject& ko, const Dpt& dpt);

    // Status-KOs (send-on-change): global "EX.1 liest aktiv" / "EX.1-API erreichbar",
    // per-channel "Letzte Abfrage" (DPT 19.001).
    bool _apiReachable = false;
    bool _lastModbusActive = false;
    bool _lastApiReachable = false;
    bool _statusSent = false;
    void updateStatusKos();

  public:
    EnergyEdgeModule();

    const std::string name() override;
    const std::string version() override;

    OpenKNX::Channel* createChannel(uint8_t _channelIndex /* used in macros, do not rename */) override;

    void setup() override;
    void loop() override;

    void showHelp() override;
    bool processCommand(const std::string cmd, bool diagnoseKo) override;

    // Flash persistence for the SolarEdge lifetime energy counter.
    uint16_t flashSize() override;
    void writeFlash() override;
    void readFlash(const uint8_t* data, const uint16_t size) override;
};

extern EnergyEdgeModule openknxEnergyEdgeModule;
