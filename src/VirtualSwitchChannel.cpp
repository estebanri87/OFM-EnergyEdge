#include "VirtualSwitchChannel.h"
#include "knxprod.h"
#include <string.h>

VirtualSwitchChannel::VirtualSwitchChannel(uint8_t channelIndex)
    : _channelIndex(channelIndex)
{
}

const std::string VirtualSwitchChannel::name()
{
    return "EnergyEdgeSwitch";
}

void VirtualSwitchChannel::setup()
{
    // deviceId aus ETS (optionales Textfeld) übernehmen, falls konfiguriert.
    const char* id = (const char*)ParamEEX_CHDeviceId;
    if (id && id[0])
    {
        strncpy(_deviceId, id, 32);
        _deviceId[32] = '\0';
    }
    logDebugP("EEX switch ch%d setup, deviceId='%s'", _channelIndex, _deviceId);
}

void VirtualSwitchChannel::loop()
{
    // Zustand kommt ereignisgetrieben über applySwitchState() aus dem Modul-Poll.
}

void VirtualSwitchChannel::applySwitchState(bool state)
{
    if (_hasState && state == _lastState)
        return; // send-on-change
    _lastState = state;
    _hasState = true;
    KoEEX_CHSchaltausgang.value(state, DPT_Switch); // DPT 1.001 an den Aktor
    logDebugP("EEX switch ch%d -> %d", _channelIndex, state ? 1 : 0);
}
