### Kategorie

Legt Richtung und Funktion des Kanals fest:

- **Energiemessung** — stellt die Wirkleistung (W) eines KNX-Verbrauchers dem EX.1 als
  Modbus-Zähler bereit (KO „Wirkleistung", DPT 14.056).
- **Smart Meter** — wie Energiemessung, zusätzlich Bezug/Einspeisung (DPT 13.013, kWh). Die
  EX.1-Sub-Meter-Maske hat kein Geräte-ID-Feld: der EX.1 fragt stets Unit-ID 1 ab; mehrere
  Smart Meter werden über die IP getrennt.
- **Schalter / Virtual Switch** — pollt den Zustand eines EX.1-Virtual-Switch über die API
  und schaltet ein KNX-Aktor-KO (DPT 1.001).
- **Wechselrichter** — stellt die Einspeisung eines PV-Wechselrichters als SolarEdge/SunSpec
  bereit; ein Wh-Lebenszähler wird intern integriert und reboot-fest gespeichert.

