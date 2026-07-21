#include "EnergyEdgeModule.h"
#include "BaseMeterChannel.h"
#include "KElectricChannel.h"
#include "SDM630Channel.h"
#include "SolarEdgeChannel.h"
#include "NetworkModule.h"
#include "knxprod.h"

EnergyEdgeModule::EnergyEdgeModule()
    : EEXChannelOwnerModule(EEX_ChannelCount)
{
}

const std::string EnergyEdgeModule::name()
{
    return "EnergyEdge";
}

const std::string EnergyEdgeModule::version()
{
#ifdef MODULE_EnergyEdgeModule_Version
    return MODULE_EnergyEdgeModule_Version;
#else
    return "";
#endif
}

OpenKNX::Channel* EnergyEdgeModule::createChannel(uint8_t _channelIndex /* used in macros, do not rename */)
{
    // Kategorie (primär, benutzerorientiert) bestimmt Verbraucher vs. Erzeuger;
    // das technische Modbus-Profil ist die Unterauswahl.
    if (ParamEEX_CHCategory == EEX_CAT_INVERTER)
        return new SolarEdgeChannel(_channelIndex); // Wechselrichter -> SunSpec

    // Verbraucher-Kategorien: KElectric (0) oder SDM630 (1).
    if (ParamEEX_CHConsumerProfile == 1)
        return new SDM630Channel(_channelIndex);
    return new KElectricChannel(_channelIndex);
}

void EnergyEdgeModule::setup()
{
    EEXChannelOwnerModule::setup(); // creates + sets up channels

    _meters.clear();
    for (uint8_t i = 0; i < getNumberOfChannels(); i++)
    {
        auto* ch = static_cast<BaseMeterChannel*>(getChannel(i));
        if (ch != nullptr)
            _meters.push_back(ch);
    }
    logInfoP("EnergyEdge: %d active meter channel(s)", (int)_meters.size());
}

void EnergyEdgeModule::loop()
{
    EEXChannelOwnerModule::loop(); // drives channel loops (energy integration etc.)

    // Modbus-Server erst starten, wenn das Netzwerk steht (einmalig).
    if (!_server.started() && openknxNetwork.established() && !_meters.empty())
        _server.start(_meters.data(), (uint8_t)_meters.size());
}

void EnergyEdgeModule::showHelp()
{
    openknx.console.printHelpLine("eex", "Show EnergyEdge Modbus meter status");
}

bool EnergyEdgeModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd != "eex")
        return false;

    logInfoP("EnergyEdge: %d meters, server %s", (int)_meters.size(),
             _server.started() ? "running" : "not started");
    for (auto* ch : _meters)
        logInfoP("  unit %d fc %d port %d -> %.1f W", ch->unitId(), ch->functionCode(),
                 ch->port(), ch->currentWatt());
    return true;
}

uint16_t EnergyEdgeModule::flashSize()
{
    return 1 + (EEX_ChannelCount * EEX_FLASH_SLOT_SIZE); // version byte + per-channel slot
}

void EnergyEdgeModule::writeFlash()
{
    openknx.flash.writeByte(1); // version
    for (uint8_t i = 0; i < EEX_ChannelCount; i++)
    {
        uint8_t slot[EEX_FLASH_SLOT_SIZE] = {0};
        auto* ch = static_cast<BaseMeterChannel*>(getChannel(i));
        if (ch != nullptr && ch->hasFlashState())
            ch->saveFlashState(slot);
        for (uint8_t b = 0; b < EEX_FLASH_SLOT_SIZE; b++)
            openknx.flash.writeByte(slot[b]);
    }
}

void EnergyEdgeModule::readFlash(const uint8_t* data, const uint16_t size)
{
    if (size == 0)
        return;
    uint8_t version = openknx.flash.readByte();
    if (version != 1)
    {
        logDebugP("EnergyEdge: unknown flash version %d", version);
        return;
    }
    uint8_t stored = (uint8_t)((size - 1) / EEX_FLASH_SLOT_SIZE);
    uint8_t count = (stored < EEX_ChannelCount) ? stored : EEX_ChannelCount;
    for (uint8_t i = 0; i < count; i++)
    {
        uint8_t slot[EEX_FLASH_SLOT_SIZE];
        for (uint8_t b = 0; b < EEX_FLASH_SLOT_SIZE; b++)
            slot[b] = openknx.flash.readByte();
        auto* ch = static_cast<BaseMeterChannel*>(getChannel(i));
        if (ch != nullptr && ch->hasFlashState())
            ch->loadFlashState(slot);
    }
}
