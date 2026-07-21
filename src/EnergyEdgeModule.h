#pragma once
#include "OpenKNX.h"
#include "ChannelOwnerModule.h"
#include "EnergyEdgeServer.h"
#include <vector>

class BaseMeterChannel;

// KNX -> Modbus-TCP server bridge. Exposes per-channel KNX power values to the
// ABB Energy Edge EX.1 via emulated Modbus meters (KElectric / SDM630 / SolarEdge).
class EnergyEdgeModule : public EEXChannelOwnerModule
{
    EnergyEdgeServer _server;
    std::vector<BaseMeterChannel*> _meters;

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
