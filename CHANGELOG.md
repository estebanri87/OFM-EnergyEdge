# Changelog OFM-EnergyEdgeModule

## 0.3.0 - 2026-07-21

### Added
- EX.1-Messwerte (EX.1 → KNX): PV-Leistung, Verbrauch, Batterie-Leistung (Entladen positiv)
  und Batterie-SoC aus der `/v2/point`-Antwort, mit zyklischem und/oder änderungsbasiertem
  Senden (Hysterese, absolut oder relativ) je Wert. Eigene ETS-Seite „EX.1 Messwerte",
  Auswahl der Werte auf der Allgemein-Seite.
- Diagnose-Befehl `eexprobe`: belegt die vom EX.1 gelesenen KElectric-Energieblöcke
  (4131..4150, 4173..4192) mit Sentinel-Werten, um die Register für Bezug/Einspeisung
  am Gerät zu ermitteln.

### Changed
- KO-Layout: 6 globale Single-KOs (Modbus-Server aktiv, EX.1-API erreichbar, 4 Messwerte);
  `KoOffset` von 705 auf 709 verschoben. **ETS-Re-Import erforderlich.**

## 0.2.0 - 2026-07-21

### Added
- Kategorie „Schalter / Virtual Switch" (EX.1 → KNX): Poll der Solar-Manager-API
  (`GET /v2/point`), Spiegelung des `switchState` auf einen KNX-Aktorkanal (DPT 1.001).
- Kategorie „Smart Meter" (getrennt von „Energiemessung").
- Status-KOs: global „Modbus-Server aktiv" und „EX.1-API erreichbar"; je Kanal optional
  „Letzte Abfrage" (DPT 19.001).

### Changed
- Kategorien bereinigt auf Energiemessung, Smart Meter, Schalter/Virtual Switch, Wechselrichter.

## 0.1.0 - 2026-07-21

### Added
- Initial Release: Modbus-TCP-Server (KNX → EX.1) für den ABB Energy Edge EX.1.
- Profile KElectric KE-P-80 (FC3, Reg 4151, kW), Eastron SDM630 (FC4, Reg 52, W),
  SolarEdge/SunSpec (Reg 0/40000, W) mit internem Wh-Lebenszähler.
- Bis zu 20 Kanäle, Unit-ID/Port je Kanal, Watchdog (Register → 0 bei ausbleibendem Wert),
  optionale Invertierung/Skalierung.
