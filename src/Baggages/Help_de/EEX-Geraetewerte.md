### Gerätewerte

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

