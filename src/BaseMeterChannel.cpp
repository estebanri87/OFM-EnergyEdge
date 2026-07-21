#include "BaseMeterChannel.h"
#include "knxprod.h"
#include <string.h>

void eexFloatToRegsBE(float value, uint16_t& highWord, uint16_t& lowWord)
{
    uint8_t host[4];
    memcpy(host, &value, 4);
    // Zieldarstellung: struct.pack(">f", value) -> be[0]=MSB .. be[3]=LSB.
    // ESP32 host ist little-endian (host[0]=LSB), daher umdrehen.
    uint8_t be[4] = {host[3], host[2], host[1], host[0]};
    highWord = (uint16_t)((be[0] << 8) | be[1]);
    lowWord = (uint16_t)((be[2] << 8) | be[3]);
}

void BaseMeterChannel::putReg(uint16_t reqAddr, uint16_t words, uint16_t* out, uint16_t regAddr, uint16_t value)
{
    if (regAddr >= reqAddr && regAddr < (uint16_t)(reqAddr + words))
        out[regAddr - reqAddr] = value;
}

BaseMeterChannel::BaseMeterChannel(uint8_t channelIndex)
    : _channelIndex(channelIndex)
{
}

const std::string BaseMeterChannel::name()
{
    return "EnergyEdge";
}

void BaseMeterChannel::setup()
{
    _category = ParamEEX_CHCategory;
    _unitId = ParamEEX_CHUnitId;
    _watchdogMs = (uint32_t)ParamEEX_CHTimeout * 1000UL;
    _invert = (bool)ParamEEX_CHInvert;
    // Optionaler Skalierungsfaktor in Promille (1000 = 1.0), 0 -> neutral.
    uint16_t scaleMilli = ParamEEX_CHScale;
    _scale = (scaleMilli == 0) ? 1.0f : (scaleMilli / 1000.0f);
    _port = isProducer() ? (uint16_t)ParamEEX_ProducerPort : (uint16_t)ParamEEX_ConsumerPort;
    logDebugP("EEX ch%d setup cat=%d unit=%d fc=%d port=%d watchdog=%lums",
              _channelIndex, _category, _unitId, functionCode(), _port, _watchdogMs);
}

void BaseMeterChannel::loop()
{
    // Basisverhalten: nichts. Ereignisgetrieben über processInputKo().
    // SolarEdge überschreibt loop() für die Energieintegration.
}

void BaseMeterChannel::processInputKo(GroupObject& ko)
{
    int index = EEX_KoCalcIndex(ko.asap());
    if (index == EEX_KoCHWirkleistung)
    {
        _watt = (float)ko.value(Dpt(14, 56));
        _lastUpdateMs = millis();
    }
}

float BaseMeterChannel::currentWatt()
{
    if (_watchdogMs > 0 && (millis() - _lastUpdateMs) > _watchdogMs)
        return 0.0f;
    float w = _watt * _scale;
    if (_invert)
        w = -w;
    return w;
}
