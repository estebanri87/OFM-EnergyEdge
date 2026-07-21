#pragma once
#include "OpenKNX.h"

// Kategorie "Schalter/Virtual Switch": Steuer-Kanal in der Gegenrichtung (EX.1 -> KNX).
// Der EX.1 hält einen "Virtual Switch" mit eigener PV-Überschuss-Logik; das Modul liest
// dessen switchState per lokaler API (Polling /v2/point) und spiegelt ihn hier auf einen
// KNX-Aktorkanal (Schaltausgang, DPT 1.001, send-on-change).
class VirtualSwitchChannel : public OpenKNX::Channel
{
    uint8_t _channelIndex;
    char _deviceId[40] = {0}; // _id des Virtual Switch aus /v2/point (optional; leer = "einziger Switch")
    bool _lastState = false;
    bool _hasState = false;

  public:
    explicit VirtualSwitchChannel(uint8_t channelIndex);

    const std::string name() override;
    void setup() override;
    void loop() override;

    // Vom Modul nach dem /v2/point-Poll aufgerufen.
    const char* deviceId() const { return _deviceId; }
    bool hasDeviceId() const { return _deviceId[0] != '\0'; }
    void applySwitchState(bool state); // sendet DPT 1.001 an den Aktor, nur bei Änderung
};
