#include "EnergyEdgeModule.h"
#include "BaseMeterChannel.h"
#include "KElectricChannel.h"
#include "SDM630Channel.h"
#include "SolarEdgeChannel.h"
#include "VirtualSwitchChannel.h"
#include "NetworkModule.h"
#include "knxprod.h"
#include <ArduinoJson.h>
#include <string.h>
#include <math.h>

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
    // Kategorie bestimmt Kanalklasse/Richtung.
    if (ParamEEX_CHCategory == EEX_CAT_SWITCH)
        return new VirtualSwitchChannel(_channelIndex); // Schalter/Virtual Switch: EX.1 -> KNX
    if (ParamEEX_CHCategory == EEX_CAT_INVERTER)
        return new SolarEdgeChannel(_channelIndex);     // Wechselrichter -> SunSpec

    // Verbraucher-Zähler: KElectric (0) oder SDM630 (1).
    if (ParamEEX_CHConsumerProfile == 1)
        return new SDM630Channel(_channelIndex);
    return new KElectricChannel(_channelIndex);
}

BaseMeterChannel* EnergyEdgeModule::meterAt(uint8_t _channelIndex /* used in macros, do not rename */)
{
    if (ParamEEX_CHCategory == EEX_CAT_SWITCH)
        return nullptr; // Schalter -> kein Modbus-Zähler
    return static_cast<BaseMeterChannel*>(getChannel(_channelIndex));
}

VirtualSwitchChannel* EnergyEdgeModule::switchAt(uint8_t _channelIndex /* used in macros, do not rename */)
{
    if (ParamEEX_CHCategory != EEX_CAT_SWITCH)
        return nullptr;
    return static_cast<VirtualSwitchChannel*>(getChannel(_channelIndex));
}

void EnergyEdgeModule::setup()
{
    EEXChannelOwnerModule::setup(); // creates + sets up channels

    _meters.clear();
    _switches.clear();
    for (uint8_t i = 0; i < getNumberOfChannels(); i++)
    {
        if (auto* m = meterAt(i))
            _meters.push_back(m);
        else if (auto* s = switchAt(i))
            _switches.push_back(s);
    }
    logInfoP("EnergyEdge: %d meter, %d switch channel(s)", (int)_meters.size(), (int)_switches.size());
}

void EnergyEdgeModule::loop()
{
    EEXChannelOwnerModule::loop(); // drives channel loops (energy integration etc.)

    // Modbus-Server erst starten, wenn das Netzwerk steht (einmalig).
    if (!_server.started() && openknxNetwork.established() && !_meters.empty())
    {
        // Upcast auf die KNX-freie Schnittstelle (BaseMeterChannel -> ModbusSource).
        std::vector<ModbusSource*> sources(_meters.begin(), _meters.end());
        _server.start(sources.data(), (uint8_t)sources.size());
    }

#ifdef OPENKNX_WEBCLIENT
    pollEx1Api();
#endif

    updateStatusKos();
}

void EnergyEdgeModule::updateStatusKos()
{
    const uint32_t WATCHDOG_MS = 30000; // "aktiv", solange innerhalb 30 s eine Abfrage kam
    uint32_t now = millis();
    bool anyRecent = false;

    for (uint8_t i = 0; i < getNumberOfChannels(); i++)
    {
        uint8_t _channelIndex = i; // für Param-/KO-Makros
        auto* m = meterAt(i);
        if (!m)
            continue;

        // Neue Modbus-Abfrage dieses Kanals -> Zeitstempel senden (falls KO aktiviert).
        if (m->consumeRequestSeen())
        {
            if ((bool)ParamEEX_CHShowLastQuery && openknx.time.isValid())
            {
                struct tm t;
                openknx.time.getLocalTime().toTm(t);
                KoEEX_CHLetzteAbfrage.value(t, DPT_DateTime);
            }
        }
        if (m->lastRequestMs() != 0 && (now - m->lastRequestMs()) < WATCHDOG_MS)
            anyRecent = true;
    }

    // Global: "EX.1 liest aktiv" (Modbus).
    if (!_statusSent || anyRecent != _lastModbusActive)
    {
        _lastModbusActive = anyRecent;
        KoEEX_ModbusActive.value(anyRecent, DPT_Switch);
    }
    // Global: "EX.1-API erreichbar" (nur relevant, wenn Schalter-Kanäle den EX.1 pollen).
    if (!_switches.empty() && (!_statusSent || _apiReachable != _lastApiReachable))
    {
        _lastApiReachable = _apiReachable;
        KoEEX_ApiReachable.value(_apiReachable, DPT_Switch);
    }
    _statusSent = true;
}

void EnergyEdgeModule::showHelp()
{
    openknx.console.printHelpLine("eex", "EnergyEdge: Status + welche Modbus-Register der EX.1 je Kanal liest");
    openknx.console.printHelpLine("eexprobe", "EnergyEdge: KElectric-Energieregister-Sentinel an/aus (Bezug/Einspeisung finden)");
}

bool EnergyEdgeModule::processCommand(const std::string cmd, bool diagnoseKo)
{
    if (cmd == "eexprobe")
    {
        bool on = !KElectricChannel::energyProbe();
        KElectricChannel::setEnergyProbe(on);
        logInfoP("EnergyEdge: Energieregister-Sentinel %s. EX.1 zeigt jetzt je Energiewert "
                 "seine Registeradresse (z.B. 4137 -> Register 4137).", on ? "AN" : "AUS");
        return true;
    }
    if (cmd != "eex")
        return false;

    logInfoP("EnergyEdge: %d meters, %d switches, server %s", (int)_meters.size(),
             (int)_switches.size(), _server.started() ? "running" : "not started");
    for (auto* ch : _meters)
    {
        logInfoP("  unit %d fc %d port %d -> %.1f W", ch->unitId(), ch->functionCode(),
                 ch->port(), ch->currentWatt());
        for (uint8_t i = 0; i < ch->reqLogCount(); i++)
            logInfoP("      EX.1 liest: addr %d cnt %d", ch->reqLogAddr(i), ch->reqLogWords(i));
    }
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
        auto* ch = meterAt(i);
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
        auto* ch = meterAt(i);
        if (ch != nullptr && ch->hasFlashState())
            ch->loadFlashState(slot);
    }
}

#ifdef OPENKNX_WEBCLIENT
#include "OpenKNX/Network/Webclient/Handler.h"

bool EnergyEdgeModule::measurementsEnabled()
{
    return (bool)ParamEEX_MvPvEnable || (bool)ParamEEX_MvConsEnable ||
           (bool)ParamEEX_MvBatEnable || (bool)ParamEEX_MvSocEnable;
}

void EnergyEdgeModule::pollEx1Api()
{
    // Der /v2/point-Poll bedient Virtual-Switch-Ausgänge UND die EX.1-Messwerte.
    if (_switches.empty() && !measurementsEnabled())
        return;
    uint16_t interval = ParamEEX_SwitchPollInterval; // Sekunden, 0 = aus
    if (interval == 0 || !openknxNetwork.established())
        return;

    uint32_t now = millis();
    if (_lastSwitchPollMs != 0 && (now - _lastSwitchPollMs) < (uint32_t)interval * 1000UL)
        return;
    _lastSwitchPollMs = now;

    const char* ip = (const char*)ParamEEX_EX1ApiIp;
    if (!ip || !ip[0])
        return;

    std::string url = "http://";
    url += ip;
    url += "/v2/point";
    openknxNetwork.webclient.get(url)
        .maxBodySize(4096)
        .onDone([this](const OpenKNX::Network::Webclient::Response& res) {
            _apiReachable = res.success();
            if (res.success())
                processPointResponse(res.body());
            else
                logDebugP("EnergyEdge: /v2/point-Poll fehlgeschlagen (status %d)", res.status());
        })
        .send();
}

// Sendet einen Messwert bei Überschreiten der Änderungsschwelle und/oder zyklisch.
// threshold==0 -> keine Änderungssendung; cyclicMs==0 -> nicht zyklisch; beide 0 -> nie.
void EnergyEdgeModule::sendMeasurement(MvState& st, bool enabled, uint8_t changeMode, float threshold,
                                       uint32_t cyclicMs, float value, GroupObject& ko, const Dpt& dpt)
{
    if (!enabled)
        return;

    bool send = false;
    if (threshold != 0.0f)
    {
        // absolut: |neu-alt|; relativ: %-Abweichung vom letzten Wert (SoC: %-Punkte vs. %-vom-Wert).
        float delta;
        if (changeMode == 1 && st.lastSent != 0.0f)
            delta = fabsf(value - st.lastSent) / fabsf(st.lastSent) * 100.0f;
        else
            delta = fabsf(value - st.lastSent);
        if (!st.sent || delta >= threshold)
            send = true;
    }
    if (cyclicMs != 0 && (!st.sent || delayCheck(st.lastSend, cyclicMs)))
        send = true;

    if (send)
    {
        ko.value(value, dpt);
        st.lastSent = value;
        st.lastSend = millis();
        st.sent = true;
    }
}

void EnergyEdgeModule::processPointResponse(const std::string& body)
{
    JsonDocument doc;
    if (deserializeJson(doc, body))
    {
        logDebugP("EnergyEdge: /v2/point JSON-Parse-Fehler");
        return;
    }

    // --- EX.1 Messwerte (Top-Level-Aggregate der Solar-Manager-API) ---
    if (measurementsEnabled())
    {
        float pW = doc["pW"] | 0.0f;   // PV-Produktion (W)
        float cW = doc["cW"] | 0.0f;   // Verbrauch (W)
        float bcW = doc["bcW"] | 0.0f; // Batterie laden (W)
        float bdW = doc["bdW"] | 0.0f; // Batterie entladen (W)
        float soc = doc["soc"] | 0.0f; // Batterie-Ladezustand (%)
        float bat = bdW - bcW;         // Entladen positiv, Laden negativ

        sendMeasurement(_mvPv, ParamEEX_MvPvEnable, ParamEEX_MvPvChangeMode, ParamEEX_MvPvThreshold,
                        ParamEEX_MvPvDelayTimeMS, pW, KoEEX_PvPower, DPT_Value_Power);
        sendMeasurement(_mvCons, ParamEEX_MvConsEnable, ParamEEX_MvConsChangeMode, ParamEEX_MvConsThreshold,
                        ParamEEX_MvConsDelayTimeMS, cW, KoEEX_Consumption, DPT_Value_Power);
        sendMeasurement(_mvBat, ParamEEX_MvBatEnable, ParamEEX_MvBatChangeMode, ParamEEX_MvBatThreshold,
                        ParamEEX_MvBatDelayTimeMS, bat, KoEEX_BatteryPower, DPT_Value_Power);
        sendMeasurement(_mvSoc, ParamEEX_MvSocEnable, ParamEEX_MvSocChangeMode, (float)ParamEEX_MvSocThreshold,
                        ParamEEX_MvSocDelayTimeMS, soc, KoEEX_BatterySoc, DPT_Scaling);
    }

    // --- Virtual-Switch-Zustände (devices-Array) ---
    if (_switches.empty())
        return;
    JsonArray arr;
    if (doc["devices"].is<JsonArray>())
        arr = doc["devices"].as<JsonArray>();
    else if (doc.is<JsonArray>())
        arr = doc.as<JsonArray>();
    else
        return;

    for (auto* sw : _switches)
    {
        bool found = false, state = false;
        for (JsonObject dev : arr)
        {
            if (dev["switchState"].isNull())
                continue; // nur Schalt-Elemente haben switchState
            int s = dev["switchState"] | 0;
            const char* id = dev["_id"];
            if (sw->hasDeviceId())
            {
                if (id && strcmp(id, sw->deviceId()) == 0)
                {
                    state = (s != 0);
                    found = true;
                    break;
                }
            }
            else
            {
                state = (s != 0); // keine ID konfiguriert -> einziges Switch-Element
                found = true;
                break;
            }
        }
        if (found)
            sw->applySwitchState(state);
    }
}
#endif // OPENKNX_WEBCLIENT
