#include <ArduinoOTA.h>
#include <Esp.h>

#include "Network.h"

#include "IO.h"
#include "RTC.h"
#include "Settings.h"

#define NOWIFI_RESTART_TIME 10 * 60 * 1000

void loopOffline();

void setup()
{
    Serial.begin(115200);
    InitIo();

    InitSettings();

    InitRTC();
    InitCurrentMeter();

    if (!ConnectWifi())
    {
        GetSmartConfigAsync();
        Serial.println("No Wifi. Starting offline loop.");
        while (true)
            loopOffline();
    }

    InitNetwork();
    SyncRTC();
    InitOTA();
}

void loop()
{
    ArduinoOTA.handle();

    if (WiFi.status() != WL_CONNECTED)
        ESP.restart();

    HandleUdpEvents();

    UpdateOutputState();

    PrintPowerUsagePeriodically();
    samplePowerUsagePeriodically();

    delayMicroseconds(50);
}

void loopOffline()
{
    if (smartConfigDone || millis() > NOWIFI_RESTART_TIME)
        ESP.restart();

    UpdateOutputState();

    PrintPowerUsagePeriodically();
    samplePowerUsagePeriodically();

    delayMicroseconds(50);
}