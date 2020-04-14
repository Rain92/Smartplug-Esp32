#pragma once

#include <Esp.h>

#include "RTC.h"
#include "Settings.h"

#define OUTPUTPIN GPIO_NUM_33
// #define OUTPUTPIN GPIO_NUM_0

void InitIo()
{
    pinMode(OUTPUTPIN, OUTPUT);
    digitalWrite(OUTPUTPIN, LOW);
}

void SetOutputState(bool state)
{
    digitalWrite(OUTPUTPIN, state);
}

void SetOutputStateTimeBased()
{
    bool on = false;
    auto time = Rtc.GetDateTime();

    auto hour = time.Hour();
    auto minute = time.Minute();
    auto day = time.DayOfWeek();

    if (time.IsValid())
    {
        for (size_t i = 0; i < NUMTIMEINTERVALS; i++)
        {
            auto &interval = settings.controlSettings.timeIntervals[i];

            if (interval.active && day == interval.weekday &&
                (hour > interval.from.hour || (hour == interval.from.hour && minute >= interval.from.minute)) &&
                (hour < interval.to.hour || (hour == interval.to.hour && minute < interval.to.minute)))
            {
                on = true;
                break;
            }
        }
    }

    SetOutputState(on);
}

void UpdateOutputState()
{
    switch (settings.controlSettings.mode)
    {
        case Off:
            SetOutputState(false);
            break;

        case On:
            SetOutputState(true);
            break;

        case TimeControlled:
            SetOutputStateTimeBased();
            break;
    }
}

void SetOutputMode(Mode mode)
{
    settings.controlSettings.mode = mode;
    SaveSettings();

    UpdateOutputState();
}
