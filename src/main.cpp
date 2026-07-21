// Standalone test harness. Excluded from the firmware build via library.json
// srcFilter ("-<main.cpp>"). The OAM firmware supplies its own main.cpp.
#include "OpenKNX.h"
#include "EnergyEdgeModule.h"

void setup()
{
    openknx.init(0);
    openknx.addModule(1, openknxEnergyEdgeModule);
    openknx.setup();
}

void loop()
{
    openknx.loop();
}
