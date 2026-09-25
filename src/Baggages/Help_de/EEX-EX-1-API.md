### EX 1-API

Verbindung zur lokalen Solar-Manager-API des EX 1 (`/v2/point`). Die Einstellungen gelten
für alle Funktionen, die Daten vom EX 1 lesen: Virtual Switch, EX 1-Messwerte und
Gerätewerte. Alle Funktionen teilen sich eine Abfrage je Intervall.

- **IP-Adresse**: IP des EX 1 im lokalen Netz.
- **Protokoll**: **HTTP** (Standard) oder **HTTPS**. Der EX 1 bietet beide parallel an,
  HTTPS jedoch nur mit TLS 1.3. Das unterstützen die TLS-Bibliotheken der OpenKNX-Hardware
  (RP2040 und ESP32) derzeit nicht – **HTTPS ist daher aktuell nicht nutzbar**, bitte HTTP
  verwenden.
- **API-Key**: Ist im EX 1 (Installateur-Portal) ein API-Key hinterlegt, verlangt die API
  ihn bei **jedem** Zugriff, auch über HTTP. Ohne passenden Key antwortet der EX 1 mit
  „401 Unauthorized" und das Statusobjekt „EX 1-API erreichbar" bleibt aus. Leer lassen,
  wenn im EX 1 kein Key hinterlegt ist.
- **Abfrageintervall** (Sekunden, 0 = aus): Abstand der API-Abfragen.

