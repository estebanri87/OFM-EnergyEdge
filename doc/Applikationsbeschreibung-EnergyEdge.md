<!-- SPDX-License-Identifier: GPL-3.0-only -->
<!-- Copyright (C) 2026 OpenKNX -->

# Applikationsbeschreibung Energy Edge Bridge

Das Modul verbindet den KNX-Bus mit dem **ABB Energy Edge EX 1** (Solar-Manager-EMS) in
beide Richtungen:

- **KNX → EX 1 (Modbus):** KNX-Leistungswerte werden als Modbus-TCP-Server bereitgestellt,
  den der EX 1 als Zähler/Wechselrichter ausliest.
- **EX 1 → KNX (API):** Der Schaltzustand eines EX 1-„Virtual Switch" sowie die
  System-Messwerte (PV, Verbrauch, Batterie) werden über die lokale Solar-Manager-API
  gelesen und auf den Bus gesendet.

Je Kanal wird über die **Kategorie** festgelegt, welche Richtung/Funktion der Kanal übernimmt.
Kanäle werden auf der Seite **Kanalauswahl** aktiviert: Nur Kanäle mit einer Kategorie
ungleich „Deaktiviert" erscheinen im ETS-Baum und werden von der Firmware angelegt.

## Wichtige Hinweise

* Diese KNXprod wird nicht von der KNX Association offiziell unterstützt!
* Die Erzeugung der KNXprod geschieht auf eure eigene Verantwortung!

# Allgemein

<!-- DOC -->
## Allgemein

Die Seite „Allgemein" bündelt die geräteweiten Einstellungen: die Modbus-Server-Ports, die
Verbindung zur EX 1-API (für Virtual Switch, Messwerte und Gerätewerte), die Auswahl der
gesendeten EX 1-Messwerte und Gerätewerte sowie die Statusobjekte.

<!-- DOCEND -->

<!-- DOC -->
## Modbus-Server-Ports

- **Modbus-Port Verbraucher** (Standard 502): Port, auf dem KElectric-/SDM630-Kanäle vom
  EX 1 gelesen werden. Mehrere Verbraucher werden über die Modbus-Unit-ID getrennt.
- **Modbus-Port Erzeuger** (Standard 1502): Port für Wechselrichter-Kanäle (SolarEdge).
  Getrennt vom Verbraucher-Port, da der EX 1 den Wechselrichter über IP/Port adressiert.

<!-- DOCEND -->

<!-- DOC -->
## EX 1-API

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

<!-- DOCEND -->

<!-- DOC -->
## Messwerte-Auswahl

Auswahl, welche der EX 1-Messwerte auf den Bus gesendet werden: PV-Leistung, Verbrauch,
Batterie-Leistung, Batterie-SoC und Netzleistung (Bezug positiv, Einspeisung negativ). Die Sende-Einstellungen je Wert stehen auf der Seite
„EX 1 Messwerte".

<!-- DOCEND -->

# Kategorien

<!-- DOC -->
## Kategorie

Legt Richtung und Funktion des Kanals fest. Die Kategorie wird in der Tabelle der Seite
**Kanalauswahl** gewählt und kann auf dem Kanal-Tab gewechselt werden; deaktiviert wird ein
Kanal nur in der Kanalauswahl.

- **Deaktiviert** — der Kanal ist ausgeschaltet, erscheint nicht im ETS-Baum und wird von
  der Firmware nicht angelegt.
- **Energiemessung** — stellt die Wirkleistung (W) eines KNX-Verbrauchers dem EX 1 als
  Modbus-Zähler bereit (KO „Wirkleistung", DPT 14.056).
- **Smart Meter** — wie Energiemessung, zusätzlich Bezug/Einspeisung (DPT 13.013, kWh). Die
  EX 1-Sub-Meter-Maske hat kein Geräte-ID-Feld: der EX 1 fragt stets Unit-ID 1 ab; mehrere
  Smart Meter werden über die IP getrennt.
- **Schalter / Virtual Switch** — pollt den Zustand eines EX 1-Virtual-Switch über die API
  und schaltet ein KNX-Aktor-KO (DPT 1.001).
- **Wechselrichter** — stellt die Einspeisung eines PV-Wechselrichters als SolarEdge/SunSpec
  bereit; ein Wh-Lebenszähler wird intern integriert und reboot-fest gespeichert.

<!-- DOCEND -->

# Kanaleinstellungen

<!-- DOC -->
## Modbus-Profil

Bestimmt den technischen Registeraufbau der Verbraucher-Kategorien:

- **KElectric KE-P-80**: FC3, Register 4151, Einheit kW, Geräte-Trennung über Unit-ID.
- **Eastron SDM630**: FC4, Register 52, Einheit W, feste Unit-ID 1 (Trennung über IP).

Wechselrichter verwenden immer das SolarEdge/SunSpec-Profil.

<!-- DOCEND -->

<!-- DOC -->
## Unit-ID

Modbus-Unit-ID (1–247), unter der der EX 1 diesen Kanal adressiert. Bei SDM630 fest 1.

<!-- DOCEND -->

<!-- DOC -->
## Watchdog

Kanal-Timeout in Sekunden (0 = aus). Bleibt der KNX-Leistungswert länger als eingestellt
aus, liefert das Modbus-Register 0, damit der EX 1 keinen veralteten Wert sieht.

<!-- DOCEND -->

<!-- DOC -->
## Aufbereitung

- **Vorzeichen invertieren**: dreht das Vorzeichen des KNX-Werts vor der Ausgabe.
- **Skalierungsfaktor**: multipliziert den KNX-Wert (z. B. Einheiten-Korrektur).

<!-- DOCEND -->

<!-- DOC -->
## SolarEdge-Identitaet

Nur Wechselrichter: **Modell** und **Seriennummer** überschreiben die SunSpec-Identität, die
der EX 1 im Identifikationsblock liest. Standardwerte sind SolarEdge-plausibel vorbelegt.

<!-- DOCEND -->

<!-- DOC -->
## Virtual-Switch-ID

Nur Schalter/Virtual Switch: die `_id` des EX 1-Virtual-Switch (aus `/v2/point`). Bleibt das
Feld leer und existiert genau ein Virtual Switch, wird dieser automatisch erkannt.

<!-- DOCEND -->

<!-- DOC -->
## Letzte-Abfrage

Blendet je Zähler-Kanal ein Status-KO ein, das den Zeitstempel der letzten Modbus-Anfrage
dieses Kanals sendet (DPT 19.001, benötigt gültige Systemzeit).

<!-- DOCEND -->

<!-- DOC -->
## EX 1-Messwerte

Auf der Seite „EX 1 Messwerte" werden je aktiviertem Wert die Sende-Einstellungen angezeigt:

- **Änderung angegeben**: absolut oder in % (relativ zum letzten Wert).
- **Senden bei Änderung um**: Schwelle; 0 = kein änderungsbasiertes Senden.
- **zyklisch senden alle**: Zeit + Zeitbasis (Sekunde/Minute/Stunde); 0 = nicht zyklisch.

Sind Schwelle und Zyklus 0, wird der Wert nicht gesendet. Die Batterie-Leistung ist beim
Entladen positiv, beim Laden negativ.

<!-- DOCEND -->

<!-- DOC -->
## Gerätewerte

Der EX 1 liefert in `/v2/point` unter `devices[]` je angeschlossenem Gerät zusätzliche
Werte. Drei davon lassen sich auf KNX-Objekte legen:

| Wert | API-Feld | DPT |
| --- | --- | --- |
| Geräteleistung | `power` | 14.056 (W) |
| Gerätetemperatur | `temperature` | 9.001 (°C) |
| Gerätestörung | `signal` | 1.005 (Alarm) |

**Bezeichnung:** Freies Textfeld, das den Namen des KOs in der Gruppenadressliste ersetzt –
nützlich, weil der voreingestellte Name (z. B. „Gerätetemperatur") nicht immer zum
tatsächlichen Gerät passt. Wird z. B. die Warmwassertemperatur eines Speichers gemessen, kann
hier „Warmwassertemperatur" eingetragen werden. Bleibt das Feld leer, wird der Standardname
verwendet.

**Geräte-ID (`_id`):** Jeder Wert hat ein **eigenes** ID-Feld, weil die Werte typischerweise
von verschiedenen Geräten stammen – etwa die Temperatur vom Warmwasserspeicher, die Leistung
vom Netzzähler. Die `_id` ist je Installation verschieden und muss von Hand eingetragen
werden. Sie steht in der Antwort der lokalen API (`GET http://<EX 1-IP>/v2/point`, siehe
EX 1-API): Im Feld `devices[]` findet sich je Eintrag `_id` zusammen mit den Werten des
Geräts. Anhand der vorhandenen Felder (z. B. `temperature`) lässt sich das gewünschte Gerät
identifizieren.

**Bleibt das ID-Feld leer, wird der Wert nicht gesendet.** Anders als beim Virtual Switch
gibt es hier keine „nimm das einzige Gerät"-Automatik, da fast alle Geräte eine `power`
liefern und die Zuordnung sonst zufällig wäre.

**Störmeldung:** Gesendet wird **1**, sobald `signal` einen anderen Wert als `connected` hat
– also bei Verbindungsverlust zum Gerät. Nur bei Änderung, ohne zyklisches Senden.

**Hinweis:** Liefert das Gerät den jeweiligen Wert in einem Poll nicht mit, bleibt der
letzte gesendete Wert auf dem Bus stehen.

<!-- DOCEND -->

# Kommunikationsobjekte

<!-- DOC -->
## Statusobjekte

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

<!-- DOCEND -->

# Diagnose

<!-- DOC -->
## Diagnose

Serielle Konsole:

- `eex`: gibt Status, Kanäle und die vom EX 1 gelesenen Register aus.
- `eexprobe`: schaltet die Energieregister-Sentinel für KElectric an/aus. Damit lassen sich
  die Register für Bezug/Einspeisung ermitteln. Jedes Float-Slot der Blöcke 4131..4150 und
  4173..4192 erhält ein Binärgewicht (Slot n = 0,01 · 2^n MWh). Der im EX 1 angezeigte
  MWh-Wert mal 100 ergibt die Binärsumme der genutzten Slots: 1=4131/4173, 2=4133/4175,
  4=4135/4177, 8=4137/4179,
  16=4139/4181, 32=4141/4183, 64=4143/4185, 128=4145/4187, 256=4147/4189, 512=4149/4191.
  Bezug wird aus Block 4131 gelesen, Einspeisung aus Block 4173.

<!-- DOCEND -->
