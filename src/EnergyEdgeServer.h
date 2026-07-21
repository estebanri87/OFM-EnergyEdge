#pragma once
#include <cstdint>
#include <vector>

class ModbusSource;
class ModbusServerTCPasync;

// Owns one eModbus ModbusServerTCPasync listener per distinct configured port and
// registers a worker per (unit-id, function code). Workers synthesize the Modbus
// response on the fly from the source's cached KNX value (event-driven, no register
// array stored). Runs fully asynchronously (AsyncTCP) - never blocks the KNX loop.
//
// This header stays KNX-free on purpose (see ModbusSource.h): eModbus is only ever
// included from EnergyEdgeServer.cpp, which must not see the KNX headers.
class EnergyEdgeServer
{
    std::vector<ModbusServerTCPasync*> _servers;
    bool _started = false;

  public:
    // Start listeners/workers for all active sources. Call once, after the network
    // link is established.
    void start(ModbusSource** sources, uint8_t count);
    bool started() const { return _started; }

  private:
    ModbusServerTCPasync* serverForPort(uint16_t port);
};
