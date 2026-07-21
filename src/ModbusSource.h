#pragma once
#include <cstdint>

// KNX-free interface so the eModbus server translation unit (EnergyEdgeServer.cpp)
// never has to include the KNX/OpenKNX headers. This is required because eModbus's
// Modbus::Error enum and the KNX stack's unscoped ComFlag::Error both leak an
// identifier named "Error" into the global scope; when both headers are visible in
// one TU the name becomes ambiguous and eModbus fails to compile. BaseMeterChannel
// implements this interface, and only ModbusSource* crosses into the eModbus TU.
class ModbusSource
{
  public:
    virtual ~ModbusSource() = default;
    virtual uint8_t unitId() const = 0;
    virtual uint16_t port() const = 0;
    virtual uint8_t functionCode() const = 0;
    virtual void buildRegisters(uint16_t reqAddr, uint16_t words, uint16_t* out) = 0;
    // Vom Worker bei jeder Modbus-Anfrage des EX.1 aufgerufen (Status-KOs + Diagnose:
    // welche Registeradressen der EX.1 tatsächlich liest).
    virtual void onModbusRequest(uint16_t addr, uint16_t words) {}
};
