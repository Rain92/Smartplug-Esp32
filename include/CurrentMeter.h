#pragma once

#include <Arduino.h>
#include <Filters.h>

#define ANALOGINPUTPIN GPIO_NUM_34

#define SLOPE 0.0182
#define OFFSET -0.1

#define SAMPLINGWINDOW 0.8

RunningStatistics statistics;

int FilteredAnalogRead(uint8_t pin, unsigned long durationus)
{
    int sumvalues = 0;
    int numsamples = 0;

    unsigned long previousMicros = micros();

    while (micros() - previousMicros < durationus)
    {
        delayMicroseconds(5);
        sumvalues += analogRead(pin);
        numsamples++;
    }

    return sumvalues / numsamples;
}

void InitCurrentMeter()
{
    pinMode(ANALOGINPUTPIN, INPUT);
    statistics.setWindowSecs(SAMPLINGWINDOW);
}

void SampleCurrent()
{
    int value = FilteredAnalogRead(ANALOGINPUTPIN, 2000);
    statistics.input(value);
}

int GetPowerUsage()
{
    float amps = OFFSET + SLOPE * statistics.sigma();
    return (int)(amps * 220);
}
