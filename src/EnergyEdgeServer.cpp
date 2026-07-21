#include "EnergyEdgeServer.h"
#include "BaseMeterChannel.h"
#include "ModbusServerTCPasync.h"

// Per-port server bookkeeping: we keep the port alongside the instance so we can
// reuse one listener for several channels (Unit-IDs) on the same port.
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

ModbusMessage makeWorker(BaseMeterChannel* ch, ModbusMessage request)
{
    uint16_t addr = 0, words = 0;
    request.get(2, addr);
    request.get(4, words);

    if (words == 0 || words > MODBUS_MAX_WORDS)
    {
        ModbusMessage err;
        err.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_VALUE);
        return err;
    }

    uint16_t regs[MODBUS_MAX_WORDS];
    ch->buildRegisters(addr, words, regs);

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
    logInfo("EnergyEdge", "Modbus TCP server listening on port %d", port);
    return server;
}

void EnergyEdgeServer::start(BaseMeterChannel** channels, uint8_t channelCount)
{
    if (_started)
        return;

    for (uint8_t i = 0; i < channelCount; i++)
    {
        BaseMeterChannel* ch = channels[i];
        if (ch == nullptr)
            continue;

        ModbusServerTCPasync* server = serverForPort(ch->port());
        server->registerWorker(ch->unitId(), (FunctionCode)ch->functionCode(),
                               [ch](ModbusMessage request) { return makeWorker(ch, request); });
        logInfo("EnergyEdge", "Registered unit %d fc %d on port %d",
                ch->unitId(), ch->functionCode(), ch->port());
    }

    _started = true;
}
