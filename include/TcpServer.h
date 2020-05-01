#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Settings.h"

#include "RTC.h"

#include "CurrentMeter.h"
#include "IO.h"
#include "Network.h"

#define TCPPORT 80

#define AKK "Success!"

AsyncWebServer server(TCPPORT);

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Invalid Command");
}

void InitTcpServer()
{
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("On command");
        SetOutputMode(On);
        request->send(200, "text/plain", AKK);
    });

    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Off command");
        SetOutputMode(Off);
        request->send(200, "text/plain", AKK);
    });

    server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Timer command");
        SetOutputMode(TimeControlled);
        request->send(200, "text/plain", AKK);
    });

    server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(settings.controlSettings.mode));
    });

    server.on("/power", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(GetPowerUsage())); });

    server.on("/powerhistory", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Get command");
        auto size = sizeof(powerUsageHistory);
        auto buffer = (uint8_t *)malloc(size);
        memcpy(buffer, (void *)&powerUsageHistory, size);
        AsyncWebServerResponse *response = request->beginResponse_P(200, "application/x-binary", buffer, size);
        request->send(response);
        free(buffer);
    });

    server.on("/getsettings", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Get command");
        auto size = sizeof(ControlSettings);
        auto buffer = (uint8_t *)malloc(size);
        memcpy(buffer, (void *)&settings.controlSettings, size);

        AsyncWebServerResponse *response = request->beginResponse_P(200, "application/x-binary", buffer, size);
        request->send(response);

        free(buffer);
    });

    server.on(
        "/setsettings", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            Serial.println("Set command");
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", AKK);
            response->addHeader("Connection", "close");
            request->send(response);
        },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            int size = sizeof(settings.controlSettings);
            if (size == len)
            {
                memcpy(&settings.controlSettings, data, size);
                SaveSettings();
                Serial.println(settings.controlSettings.timeIntervals[23].from.hour);
            }
            else
            {
                Serial.println("Invalid Data length!");
            }
        });

    server.on("/setturnoffcountdown", HTTP_POST, [](AsyncWebServerRequest *request) {
        Serial.println("Turn off countdown command");
        if (request->hasParam("time", true))
        {
            int seconds = request->getParam("time", true)->value().toInt();
            if (seconds > 0)
            {
                Serial.printf("Turning off in %d seconds. \n", seconds);
                turnOffCountdownEnd = millis() + seconds * 1000;
                turnOffCountdownActive = true;
            }
            else
            {
                turnOffCountdownActive = false;
            }
        }
        request->send(200, "text/plain", AKK);
    });

    server.on("/getturnoffcountdown", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain",
                      turnOffCountdownActive ? String((turnOffCountdownEnd - millis()) / 1000) : String("0"));
    });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", AKK);
        ESP.restart();
    });

    server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
        auto now = Rtc.GetDateTime();
        request->send(200, "text/plain", String(now.Hour()) + ':' + now.Minute() + ':' + now.Second());
    });

    server.on("/mem", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Mem command: " + String(ESP.getFreeHeap()));
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.onNotFound(notFound);

    server.begin();
}