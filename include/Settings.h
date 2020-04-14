#pragma once

#include <ArduinoNvs.h>

#include "RtcDS3231.h"

#define MAGICKEY 836457362ull

#define MAGICKEY_FLASHKEY "magickey"
#define SETTINGS_FLASHKEY "settings"

#define WIFISSID_DEFAULT "WLAN-66FB40_EXT"
#define WIFIPASSWORD_DEFAULT "dontpublishthis"
#define NETID_DEFAULT "esp32"

#define MODE_DEFAULT Off

#define NUMTIMEINTERVALS 32

struct WiFiConfig
{
    char ssid[32] = WIFISSID_DEFAULT;
    char password[32] = WIFIPASSWORD_DEFAULT;
};

struct Time
{
    int hour;
    int minute;
};

struct TimeInterval
{
    bool active = false;
    Time from;
    Time to;
    DayOfWeek weekday;
};

enum Mode
{
    Off = 0,
    On,
    TimeControlled
};

struct ControlSettings
{
    char netId[32] = NETID_DEFAULT;
    Mode mode = MODE_DEFAULT;
    TimeInterval timeIntervals[NUMTIMEINTERVALS];
};

struct Settings
{
    ControlSettings controlSettings;
    WiFiConfig wificonfig;
};

Settings settings;

bool SaveSettings()
{
    Serial.println("Saving settings.");

    bool res1 = NVS.setInt(MAGICKEY_FLASHKEY, MAGICKEY);
    bool res2 = NVS.setBlob(SETTINGS_FLASHKEY, (uint8_t *)&settings, sizeof(Settings));

    if (!res1 || !res2)
        Serial.println("Couldn't save settings.");

    return res1 && res2;
}

void PrintSettings()
{
    Serial.println(settings.wificonfig.ssid);
    Serial.println(settings.wificonfig.password);
}

bool InitSettings()
{
    NVS.begin();

    uint64_t magickey = NVS.getInt(MAGICKEY_FLASHKEY);
    size_t blobsize = NVS.getBlobSize(SETTINGS_FLASHKEY);

    if (magickey != MAGICKEY || blobsize != sizeof(Settings))
    {
        NVS.eraseAll();
        Serial.println(blobsize);
        SaveSettings();
    }
    else
    {
        Serial.println("Loading settings.");

        bool res = NVS.getBlob(SETTINGS_FLASHKEY, (uint8_t *)&settings, sizeof(Settings));
        if (!res)
            Serial.println("Couldn't load settings.");
        PrintSettings();
    }

    return true;
}