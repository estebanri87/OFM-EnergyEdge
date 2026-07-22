# Changelog OFM-EnergyEdgeModule

## 0.5.0 - 2026-07-22

### Added
- Kategorie **Smart Meter**: zwei KNX-Eingangs-KOs **Bezug** und **Einspeisung** (DPT 13.013,
  kWh) werden dem EX.1 als Energiezählerstände bereitgestellt. Die Werte gehen ohne Umrechnung
  durch — der EX.1 liest die Energieregister in kWh, anders als die Leistung in kW.

### Changed
- KO-Blockgröße je Kanal von 3 auf 5 erhöht. **Verschiebt alle Kanal-KO-Nummern,
  ETS-Re-Import und erneuter Download erforderlich.**

### Notes
- Die Zielregister wurden am realen Gerät per `eexprobe` ermittelt: Der EX.1 liest die Blöcke
  4131..4150 und 4173..4192 und **summiert je Wert zwei Register** (Slot 0 und Slot 8, also
  Bezug 4131+4147, Einspeisung 4173+4189 — typisch für einen Zweitarifzähler). Das Modul legt
  den vollen Wert auf das jeweils erste Register; das zweite bleibt 0, die Summe stimmt damit.
- Energie hat bewusst **keinen Watchdog**: Ein Zählerstand darf nicht auf 0 fallen, nur weil
  gerade kein Telegramm kam.

## 0.4.0 - 2026-07-22

### Added
- Die globalen Status-KOs „Modbus-Server aktiv" und „EX.1-API erreichbar" lassen sich per
  ETS-Checkbox einzeln ein-/ausblenden; ausgeblendete Objekte werden auch nicht gesendet.

### Changed
- `eexprobe` verwendet Binärgewichte (Slot n = 0,01 · 2^n MWh) statt der Registeradresse als
  Markerwert. Der EX.1 summiert je Energiewert zwei Register — mit Adressen als Marker war die
  Summe symmetrisch und damit mehrdeutig, mit Binärgewichten ist sie eindeutig dekodierbar.

### Fixed
- Kanäle jenseits von „Aktive Kanäle" wurden zur Laufzeit trotzdem angelegt und
  registrierten Modbus-Worker auf der ungültigen Unit-ID 0. `createChannel()` liefert für
  diese Kanäle jetzt `nullptr`, sie belegen weder Speicher noch Modbus-Worker.
  **Hinweis:** „Aktive Kanäle" muss die tatsächlich genutzte Kanalzahl abdecken, sonst
  fallen zuvor per Restspeicher noch laufende Kanäle aus.

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
