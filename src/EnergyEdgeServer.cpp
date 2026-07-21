// NOTE: This translation unit deliberately does NOT include any KNX/OpenKNX header.
// eModbus's Modbus::Error and the KNX stack's ComFlag::Error collide (both leak an
// "Error" identifier), so eModbus and KNX must never share a TU. The rest of the
// module talks to us only through the KNX-free ModbusSource interface.
#include <Arduino.h>
#include "ModbusServerTCPasync.h"
#include "EnergyEdgeServer.h"
#include "ModbusSource.h"

namespace
{
struct PortServer
{
    uint16_t port;
    ModbusServerTCPasync* server;
};
std::vector<PortServer> g_portServers;

// Modbus max registers per read response (FC3/FC4).
constexpr uint16_t MODBUS_MAX_WORDS = 125;

ModbusMessage makeWorker(ModbusSource* source, ModbusMessage request)
{
    uint16_t addr = 0, words = 0;
    request.get(2, addr);
    request.get(4, words);

    // Diagnose: zeigt exakt, welche Unit-ID / Register der EX.1 abfragt.
    log_i("EnergyEdge req: unit=%d fc=%d addr=%d words=%d",
          request.getServerID(), request.getFunctionCode(), addr, words);
    source->onModbusRequest(addr, words); // Status-KOs + Registermitschnitt

    if (words == 0 || words > MODBUS_MAX_WORDS)
    {
        ModbusMessage err;
        err.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
        return err;
    }

    uint16_t regs[MODBUS_MAX_WORDS];
    source->buildRegisters(addr, words, regs);

    ModbusMessage response;
    response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
    for (uint16_t i = 0; i < words; i++)
        response.add(regs[i]);
    return response;
}
} // namespace

ModbusServerTCPasync* EnergyEdgeServer::serverForPort(uint16_t port)
{
    for (auto& ps : g_portServers)
        if (ps.port == port)
            return ps.server;

    auto* server = new ModbusServerTCPasync();
    // maxClients 4, idle timeout 20 s. Async -> no loop() polling required.
    server->start(port, 4, 20000);
    g_portServers.push_back({port, server});
    _servers.push_back(server);
    log_i("EnergyEdge: Modbus TCP server listening on port %d", port);
    return server;
}

void EnergyEdgeServer::start(ModbusSource** sources, uint8_t count)
{
    if (_started)
        return;

    for (uint8_t i = 0; i < count; i++)
    {
        ModbusSource* source = sources[i];
        if (source == nullptr)
            continue;

        ModbusServerTCPasync* server = serverForPort(source->port());
        server->registerWorker(source->unitId(), (FunctionCode)source->functionCode(),
                               [source](ModbusMessage request) { return makeWorker(source, request); });
        log_i("EnergyEdge: registered unit %d fc %d on port %d",
              source->unitId(), source->functionCode(), source->port());
    }

    _started = true;
}
