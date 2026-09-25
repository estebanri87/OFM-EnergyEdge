# OFM-EnergyEdgeModule

OpenKNX-Modul zur bidirektionalen Anbindung des **ABB Energy Edge EX.1** (Solar-Manager-EMS)
an den KNX-Bus. Das Modul stellt KNX-Messwerte als **Modbus-TCP-Server** bereit, den der EX.1
ausliest, und spiegelt umgekehrt EX.1-„Virtual Switches" sowie System-Messwerte (PV, Verbrauch,
Batterie) auf den KNX-Bus. 

## Release Notes

Die vollständige Historie liegt in [CHANGELOG.md](CHANGELOG.md).

- 0.8.0 Kanalauswahl nach OpenKNX-Standard (Kategorie mit „Deaktiviert", **nicht abwärtskompatibel**), EX 1-API: API-Key auch über HTTP, Voreinstellung HTTP, Verbindungseinstellungen allgemein benannt
- 0.7.0 Netzleistung und Gerätewerte (Leistung, Temperatur, Störung) aus `devices[]`
- 0.6.0 EX 1-API: Protokoll HTTP/HTTPS und API-Key
- 0.5.0 Smart Meter: Bezug und Einspeisung (DPT 13.013) an den EX.1
- 0.4.0 Status-KOs ausblendbar, Binärgewichte im Energieregister-Probe, Fix für Kanäle jenseits von „Aktive Kanäle"
- 0.3.0 EX.1-Messwerte (PV/Verbrauch/Batterie/SoC → KNX), Sentinel-Diagnose für Smart-Meter-Energieregister
- 0.2.0 Virtual Switch (EX.1 → KNX), Status-KOs, Kategorie „Smart Meter"
- 0.1.0 Initial Release: Modbus-TCP-Server (KElectric / SDM630 / SolarEdge SunSpec)

## Abhängigkeiten

- [OFM-Network](https://github.com/OpenKNX/OFM-Network) für die Netzwerkanbindung. Für den
  EX.1-API-Zugriff (Virtual Switch, EX.1-Messwerte) muss der Webclient aktiv sein
  (`-D OPENKNX_WEBCLIENT`).
- [eModbus](https://github.com/eModbus/eModbus) (`miq19/eModbus`) für den Modbus-TCP-Server.
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson) für das Parsen der Solar-Manager-API.

## Funktionen

Zweistufiges Kanalmodell: **Kategorie** (primär) + **Modbus-Profil** (technischer Registeraufbau).

### Kategorien

- **Energiemessung** — Wirkleistung (W) eines KNX-Verbrauchers an den EX.1.
- **Smart Meter** — zusätzlich Bezug und Einspeisung (DPT 13.013, kWh). Der EX.1 liest die
  Energie in kWh, also ohne Umrechnung — anders als die Leistung, die er in kW erwartet.
- **Schalter / Virtual Switch** — der EX.1 schaltet (PV-Überschuss-Logik im EX.1); der Zustand
  wird über die Solar-Manager-API (`GET /v2/point`) gepollt und auf einen KNX-Aktorkanal
  (DPT 1.001) gespiegelt.
- **Wechselrichter** — Einspeisung eines PV-Wechselrichters als SolarEdge/SunSpec-Erzeuger.

### Modbus-Profile (KNX → EX.1)

| Profil               | FC  | Register | Einheit | Geräte-Trennung |
|----------------------|-----|----------|---------|-----------------|
| KElectric KE-P-80    | 3   | 4151     | kW      | Unit-ID         |
| Eastron SDM630       | 4   | 52       | W       | IP (Unit-ID 1)  |
| SolarEdge (SunSpec)  | 3   | 0 / 40000| W (SF1) | IP / Port       |

- Wirkleistung je Kanal als KNX-Eingang (DPT 14.056, Watt).
- Watchdog je Kanal: bleibt der KNX-Wert aus, liefert das Register 0.
- SolarEdge integriert intern einen Wh-Lebenszähler (reboot-fest über Flash).

### EX.1-Messwerte (EX.1 → KNX)

Aus derselben `/v2/point`-Abfrage werden vier System-Werte auf den Bus gesendet, mit
OpenKNX-Standard-Sendeverhalten (zyklisch und/oder bei Änderung mit Hysterese):
PV-Leistung, Verbrauch, Batterie-Leistung (Entladen positiv) und Batterie-SoC.

### Status-KOs

- **Modbus-Server aktiv** (global) — der EX.1 liest aktiv per Modbus.
- **EX.1-API erreichbar** (global) — der `/v2/point`-Poll ist erfolgreich.
- **Letzte Abfrage** (je Kanal, optional) — Zeitstempel der letzten Modbus-Anfrage (DPT 19.001).

### Diagnose (Serial-Konsole)

- `eex` — Status, und welche Register der EX.1 je Kanal liest.
- `eexprobe` — belegt die KElectric-Energieblöcke mit Sentinel-Werten, um die Register für
  Bezug/Einspeisung zu ermitteln.

## Hardware-Unterstützung

|Prozessor | Status | Anmerkung                       |
|----------|--------|---------------------------------|
|ESP32     | Beta   | Modbus-TCP-Server über lwIP/ETH |
|RP2040    | -      | eModbus/AsyncTCP nicht getestet |

Getestete Hardware:
- [OpenKNX REG1 Basismodul LAN+TP](https://github.com/OpenKNX/OpenKNX/wiki/REG1-Eth)

## Einbindung in die Anwendung

In das Anwendungs-XML muss das OFM-EnergyEdgeModule aufgenommen werden:

```xml
  <op:define prefix="EEX" ModuleType="27"
    share=   "../lib/OFM-EnergyEdgeModule/src/EnergyEdgeModule.share.xml"
    template="../lib/OFM-EnergyEdgeModule/src/EnergyEdgeModule.templ.xml"
    NumChannels="20"
    KoSingleOffset="703"
    KoOffset="709">
    <op:verify File="../lib/OFM-EnergyEdgeModule/library.json" ModuleVersion="0.8" />
  </op:define>
```

**Hinweis:** `ModuleType`, `KoSingleOffset`, `KoOffset` und `NumChannels` müssen je nach
Anwendung angepasst werden. Das Modul belegt 6 globale (Single-)KOs und 5 KOs je Kanal.

In `main.cpp` muss das Modul ebenfalls hinzugefügt werden:

```cpp
[...]
#include "EnergyEdgeModule.h"
[...]

void setup()
{
    [...]
    openknx.addModule(1, openknxNetwork);
    openknx.addModule(10, openknxEnergyEdgeModule);
    [...]
}
```

Zusätzlich im ESP32-Build (`platformio.ini`) den Webclient aktivieren:

```ini
build_flags =
    -D OPENKNX_WEBCLIENT
lib_ignore =
    ESPAsyncTCP
```

## KO-Präfix

`EEX`

## Lizenz

[GNU GPL v3](LICENSE)
