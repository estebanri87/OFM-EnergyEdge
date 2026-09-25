# Changelog OFM-EnergyEdgeModule

## 0.8.0 - 2026-09-25

### Breaking
- **Kanalauswahl nach OpenKNX-Standard.** Der Schieberegler „Aktive Kanäle" und der Tab
  „(mehr)" entfallen. Kanäle werden auf der neuen Seite **Kanalauswahl** über die Kategorie
  aktiviert. Die Kategorie hat dafür den neuen Wert **0 = Deaktiviert**, die bisherigen Werte
  rücken um eins nach oben (1 = Energiemessung, 2 = Smart Meter, 3 = Schalter/Virtual Switch,
  4 = Wechselrichter). **Bestehende Projekte: Applikation in der ETS aktualisieren, je Kanal
  die Kategorie in der Kanalauswahl neu setzen und das Gerät neu programmieren.**

### Added
- Seite **Kanalauswahl** (je Kanal eine Tabellenzeile: Kanal, Kategorie, Beschreibung).
  Deaktivierte Kanäle erscheinen nicht im ETS-Baum und werden von der Firmware nicht
  angelegt. Die Kategorie lässt sich zusätzlich auf dem Kanal-Tab wechseln (synchronisiert
  über `BASE_SyncChannelType`).
- Hilfetexte werden aus der Applikationsbeschreibung erzeugt: VS-Code-Task
  „OpenKNXproducer Documentation" (`.vscode/tasks.json`).

### Changed
- Seite „Allgemein": Die Verbindungseinstellungen stehen unter **„EX 1-API (Verbindung)"**
  statt „Virtual Switch", da sie für Virtual Switch, Messwerte und Gerätewerte gelten. Das
  Intervall heißt **„Abfrageintervall"**.
- Protokoll-Voreinstellung ist jetzt **HTTP**. Der EX 1 nimmt HTTPS nur mit TLS 1.3 an; das
  unterstützen weder RP2040 (BearSSL) noch ESP32 (mbedTLS) derzeit.
- Das Feld **API-Key** ist bei beiden Protokollen sichtbar.
- Kanal-Tab beginnt mit „Kanaldefinition" (Beschreibung, Kategorie); das Feld „Bezeichnung"
  heißt jetzt „Beschreibung".

### Fixed
- **Keine Verbindung zur EX 1-API über HTTP:** Der Header `x-api-key` wurde nur bei HTTPS
  gesendet. Ist im EX 1 ein API-Key hinterlegt, verlangt er ihn aber auch über HTTP und
  antwortet sonst mit 401.
- Statusobjekt „EX 1-API erreichbar" wird jetzt auch aktualisiert, wenn nur Messwerte oder
  Gerätewerte (ohne Virtual-Switch-Kanal) die API nutzen.

## 0.7.0 - 2026-07-31

### Added
- **Netzleistung** (Bezug positiv, Einspeisung negativ), abgeleitet aus Verbrauch,
  PV-Leistung und Batterie-Leistung.
- **Gerätewerte** aus `devices[]` der EX 1-API: Geräteleistung, Gerätetemperatur und
  Gerätestörung, je Wert mit eigener Geräte-ID (`_id`).

### Changed
- Schreibweise „EX 1" statt „EX.1" in den Hilfetexten.

## 0.6.0 - 2026-07-24

### Added
- EX 1-API: Auswahl des **Protokolls** (HTTP/HTTPS) und Feld für den **API-Key**
  (Header `x-api-key`).

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
- **Am Gerät verifiziert (2026-07-22):** Mit dem realen Zählerstand der Wärmepumpe
  (13.967 kWh) zeigt der EX.1 den Bezug korrekt als 14 MWh an; die Einspeisung bleibt 0 und
  wird vom EX.1 dann ausgeblendet. Die per `eexprobe` ermittelte Registerzuordnung stimmt
  damit auch im praktischen Betrieb.

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
