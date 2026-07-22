<!-- SPDX-License-Identifier: GPL-3.0-only -->
<!-- Copyright (C) 2026 OpenKNX -->

# Applikationsbeschreibung Energy Edge Bridge

Das Modul verbindet den KNX-Bus mit dem **ABB Energy Edge EX.1** (Solar-Manager-EMS) in
beide Richtungen:

- **KNX → EX.1 (Modbus):** KNX-Leistungswerte werden als Modbus-TCP-Server bereitgestellt,
  den der EX.1 als Zähler/Wechselrichter ausliest.
- **EX.1 → KNX (API):** Der Schaltzustand eines EX.1-„Virtual Switch" sowie die
  System-Messwerte (PV, Verbrauch, Batterie) werden über die lokale Solar-Manager-API
  gelesen und auf den Bus gesendet.

Je Kanal wird über die **Kategorie** festgelegt, welche Richtung/Funktion der Kanal übernimmt.

## Wichtige Hinweise

* Diese KNXprod wird nicht von der KNX Association offiziell unterstützt!
* Die Erzeugung der KNXprod geschieht auf eure eigene Verantwortung!

# Allgemein

<!-- DOC -->
## Allgemein

Die Seite „Allgemein" bündelt die geräteweiten Einstellungen: die Modbus-Server-Ports, den
Zugang zur EX.1-API (für Virtual Switch und Messwerte), die Anzahl aktiver Kanäle sowie die
Auswahl der gesendeten EX.1-Messwerte.

<!-- DOCEND -->

<!-- DOC -->
## Modbus-Server-Ports

- **Modbus-Port Verbraucher** (Standard 502): Port, auf dem KElectric-/SDM630-Kanäle vom
  EX.1 gelesen werden. Mehrere Verbraucher werden über die Modbus-Unit-ID getrennt.
- **Modbus-Port Erzeuger** (Standard 1502): Port für Wechselrichter-Kanäle (SolarEdge).
  Getrennt vom Verbraucher-Port, da der EX.1 den Wechselrichter über IP/Port adressiert.

<!-- DOCEND -->

<!-- DOC -->
## EX.1-API

- **EX.1 API IP-Adresse**: IP des EX.1, unter der die Solar-Manager-API (`/v2/point`)
  erreichbar ist. Nötig für Virtual Switch und EX.1-Messwerte.
- **Virtual-Switch Poll-Intervall** (Sekunden, 0 = aus): Abstand der API-Abfragen.

<!-- DOCEND -->

<!-- DOC -->
## Kanalauswahl

**Aktive Kanäle** (0–20): Anzahl der eingeblendeten Kanäle. Jeder Kanal bildet ein
Modbus-Gerät (Zähler/Wechselrichter) oder einen Virtual Switch ab.

<!-- DOCEND -->

<!-- DOC -->
## Messwerte-Auswahl

Auswahl, welche der vier EX.1-Messwerte auf den Bus gesendet werden: PV-Leistung, Verbrauch,
Batterie-Leistung und Batterie-SoC. Die Sende-Einstellungen je Wert stehen auf der Seite
„EX.1 Messwerte".

<!-- DOCEND -->

# Kategorien

<!-- DOC -->
## Kategorie

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

Modbus-Unit-ID (1–247), unter der der EX.1 diesen Kanal adressiert. Bei SDM630 fest 1.

<!-- DOCEND -->

<!-- DOC -->
## Watchdog

Kanal-Timeout in Sekunden (0 = aus). Bleibt der KNX-Leistungswert länger als eingestellt
aus, liefert das Modbus-Register 0, damit der EX.1 keinen veralteten Wert sieht.

<!-- DOCEND -->

<!-- DOC -->
## Aufbereitung

- **Vorzeichen invertieren**: dreht das Vorzeichen des KNX-Werts vor der Ausgabe.
- **Skalierungsfaktor**: multipliziert den KNX-Wert (z. B. Einheiten-Korrektur).

<!-- DOCEND -->

<!-- DOC -->
## SolarEdge-Identitaet

Nur Wechselrichter: **Modell** und **Seriennummer** überschreiben die SunSpec-Identität, die
der EX.1 im Identifikationsblock liest. Standardwerte sind SolarEdge-plausibel vorbelegt.

<!-- DOCEND -->

<!-- DOC -->
## Virtual-Switch-ID

Nur Schalter/Virtual Switch: die `_id` des EX.1-Virtual-Switch (aus `/v2/point`). Bleibt das
Feld leer und existiert genau ein Virtual Switch, wird dieser automatisch erkannt.

<!-- DOCEND -->

<!-- DOC -->
## Letzte-Abfrage

Blendet je Zähler-Kanal ein Status-KO ein, das den Zeitstempel der letzten Modbus-Anfrage
dieses Kanals sendet (DPT 19.001, benötigt gültige Systemzeit).

<!-- DOCEND -->

<!-- DOC -->
## EX.1-Messwerte

Auf der Seite „EX.1 Messwerte" werden je aktiviertem Wert die Sende-Einstellungen angezeigt:

- **Änderung angegeben**: absolut oder in % (relativ zum letzten Wert).
- **Senden bei Änderung um**: Schwelle; 0 = kein änderungsbasiertes Senden.
- **zyklisch senden alle**: Zeit + Zeitbasis (Sekunde/Minute/Stunde); 0 = nicht zyklisch.

Sind Schwelle und Zyklus 0, wird der Wert nicht gesendet. Die Batterie-Leistung ist beim
Entladen positiv, beim Laden negativ.

<!-- DOCEND -->

# Kommunikationsobjekte

<!-- DOC -->
## Statusobjekte

- **Modbus-Server aktiv** (DPT 1.011): der EX.1 liest aktiv per Modbus.
- **EX.1-API erreichbar** (DPT 1.011): der `/v2/point`-Poll ist erfolgreich.

Beide Status-KOs lassen sich auf der Seite „Allgemein" per Checkbox ein-/ausblenden, falls
sie nicht benötigt werden.
- **PV-Leistung / Verbrauch / Batterie-Leistung** (DPT 14.056, W) und
  **Batterie-SoC** (DPT 5.001, %): die EX.1-Messwerte.

Kanal-Objekte: **Wirkleistung** (DPT 14.056, Eingang), **Schaltausgang** (DPT 1.001, Ausgang)
und optional **Letzte Abfrage** (DPT 19.001).

<!-- DOCEND -->

# Diagnose

<!-- DOC -->
## Diagnose

Serielle Konsole:

- `eex`: gibt Status, Kanäle und die vom EX.1 gelesenen Register aus.
- `eexprobe`: schaltet die Energieregister-Sentinel für KElectric an/aus. Damit lassen sich
  die Register für Bezug/Einspeisung ermitteln. Jedes Float-Slot der Blöcke 4131..4150 und
  4173..4192 erhält ein Binärgewicht (Slot n = 0,01 · 2^n MWh). Der im EX.1 angezeigte
  MWh-Wert mal 100 ergibt die Binärsumme der genutzten Slots: 1=4131/4173, 2=4133/4175,
  4=4135/4177, 8=4137/4179,
  16=4139/4181, 32=4141/4183, 64=4143/4185, 128=4145/4187, 256=4147/4189, 512=4149/4191.
  Bezug wird aus Block 4131 gelesen, Einspeisung aus Block 4173.

<!-- DOCEND -->
