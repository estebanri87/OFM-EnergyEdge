### Statusobjekte

- **Modbus-Server aktiv** (DPT 1.011): der EX 1 liest aktiv per Modbus.
- **EX 1-API erreichbar** (DPT 1.011): die `/v2/point`-Abfrage ist erfolgreich (nur aktiv,
  wenn Virtual Switch, Messwerte oder Gerätewerte die API nutzen).

Beide Status-KOs lassen sich auf der Seite „Allgemein" per Checkbox ein-/ausblenden, falls
sie nicht benötigt werden.
- **PV-Leistung / Verbrauch / Batterie-Leistung** (DPT 14.056, W) und
  **Batterie-SoC** (DPT 5.001, %): die EX 1-Messwerte.
- **Geräteleistung** (DPT 14.056, W), **Gerätetemperatur** (DPT 9.001, °C) und
  **Gerätestörung** (DPT 1.005): die Gerätewerte aus `devices[]` (siehe oben).

Kanal-Objekte: **Wirkleistung** (DPT 14.056, Eingang), **Schaltausgang** (DPT 1.001, Ausgang)
und optional **Letzte Abfrage** (DPT 19.001).

