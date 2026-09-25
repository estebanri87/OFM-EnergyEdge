### Diagnose

Serielle Konsole:

- `eex`: gibt Status, Kanäle und die vom EX 1 gelesenen Register aus.
- `eexprobe`: schaltet die Energieregister-Sentinel für KElectric an/aus. Damit lassen sich
  die Register für Bezug/Einspeisung ermitteln. Jedes Float-Slot der Blöcke 4131..4150 und
  4173..4192 erhält ein Binärgewicht (Slot n = 0,01 · 2^n MWh). Der im EX 1 angezeigte
  MWh-Wert mal 100 ergibt die Binärsumme der genutzten Slots: 1=4131/4173, 2=4133/4175,
  4=4135/4177, 8=4137/4179,
  16=4139/4181, 32=4141/4183, 64=4143/4185, 128=4145/4187, 256=4147/4189, 512=4149/4191.
  Bezug wird aus Block 4131 gelesen, Einspeisung aus Block 4173.

