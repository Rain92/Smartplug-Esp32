#pragma once

#include <Arduino.h>
#include "esp_timer.h"

#define ANALOGINPUTPIN GPIO_NUM_36

#define SLOPE 11.638
#define OFFSET 0.0

#define SIGMANUMSAMPLES 40
#define CURRENTNUMSAMPLES 9

#define POWERUSAGEHISTORYSIZE 120

#define POWERUSAGEPRINTPERIOD 1000
#define POWERUSAGESAMPLEPERIOD 1000

int powerUsageHistory[POWERUSAGEHISTORYSIZE];

int FilteredAnalogRead(uint8_t pin, unsigned long durationus)
{
    int sumvalues = 0;
    int numsamples = 0;

    unsigned long previousMicros = micros();

    while (micros() - previousMicros < durationus)
    {
        delayMicroseconds(1);
        sumvalues += analogRead(pin);
        numsamples++;
    }

    return sumvalues / numsamples;
}

float SampleSigma()
{
    float average = 0;
    float averagesqr = 0;

    for (int i = 0; i < SIGMANUMSAMPLES; i++)
    {
        int value = FilteredAnalogRead(ANALOGINPUTPIN, 2000);
        average += value;
        averagesqr += value * value;
    }
    average /= SIGMANUMSAMPLES;
    averagesqr /= SIGMANUMSAMPLES;

    float sigma = sqrt(max(0.0f, averagesqr - average * average));

    return sigma;
}

float SampleCurrent()
{
    float sigmas[CURRENTNUMSAMPLES];
    for (int c = 0; c < CURRENTNUMSAMPLES; c++)
        sigmas[c] = SampleSigma();

    qsort(sigmas, CURRENTNUMSAMPLES, sizeof(sigmas[0]),
          [](const void *a, const void *b) { return *((int *)a) - *((int *)b); });

    return sigmas[CURRENTNUMSAMPLES / 2 - 1];
}

void SamplePowerUsage()
{
    float current = SampleCurrent();
    int power = (int)(OFFSET + SLOPE * current);

    for (size_t i = POWERUSAGEHISTORYSIZE - 1; i > 0; i--)
        powerUsageHistory[i] = powerUsageHistory[i - 1];

    powerUsageHistory[0] = power;
}

int GetPowerUsage()
{
    return powerUsageHistory[0];
}

void InitCurrentMeter()
{
    pinMode(ANALOGINPUTPIN, INPUT);
    adcAttachPin(ANALOGINPUTPIN);

    for (size_t i = 0; i < POWERUSAGEHISTORYSIZE; i++)
        powerUsageHistory[i] = 0;
}

unsigned long previousPrintMillis = 0;
void PrintPowerUsagePeriodically()
{
    if ((unsigned long)(millis() - previousPrintMillis) >= POWERUSAGEPRINTPERIOD)
    {
        previousPrintMillis = millis();
        Serial.print("Power Usage(Watt): ");
        Serial.println(GetPowerUsage());
    }
}

unsigned long previousSampleMiliis = 0;
void samplePowerUsagePeriodically()
{
    if ((unsigned long)(millis() - previousSampleMiliis) >= POWERUSAGESAMPLEPERIOD)
    {
        previousSampleMiliis = millis();
        SamplePowerUsage();
    }
}
