#pragma once

#include <Wire.h>
#include <ezTime.h>

#include "Network.h"
#include "RtcDS3231.h"

#define TIMEZONE "Europe/Berlin"

RtcDS3231<TwoWire> Rtc(Wire);

void InitRTC()
{
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (!Rtc.IsDateTimeValid() || now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }

    Rtc.Enable32kHzPin(false);

    Rtc.LatchAlarmsTriggeredFlags();
}

RtcDateTime GetNTPTime()
{
    Serial.println("Getting Time. Waiting for sync");

    waitForSync(5);

    Timezone timezone;
    timezone.setLocation(TIMEZONE);

    Serial.println("Time:");
    Serial.println(timezone.dateTime());

    RtcDateTime rtcdatetime(timezone.year(), timezone.month(), timezone.day(), timezone.hour(), timezone.minute(),
                            timezone.second());

    return rtcdatetime;
}

void SyncRTC()
{
    Rtc.SetDateTime(GetNTPTime());
}