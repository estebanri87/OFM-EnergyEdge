#pragma once
#include "OpenKNX.h"
#include <vector>

class BaseMeterChannel;
class ModbusServerTCPasync;

// Owns one eModbus ModbusServerTCPasync listener per distinct configured port and
// registers a worker per (unit-id, function code). Workers synthesize the Modbus
// response on the fly from the channel's cached KNX value (event-driven, no register
// array stored). Runs fully asynchronously (AsyncTCP) - never blocks the KNX loop.
class EnergyEdgeServer
{
    std::vector<ModbusServerTCPasync*> _servers;
    bool _started = false;

  public:
    // Start listeners/workers for all active channels. Call once, after the network
    // link is established.
    void start(BaseMeterChannel** channels, uint8_t channelCount);
    bool started() const { return _started; }

  private:
    ModbusServerTCPasync* serverForPort(uint16_t port);
};
