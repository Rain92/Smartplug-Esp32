#pragma once

#include <ArduinoOTA.h>
#include <Esp.h>
#include <WiFi.h>
#include <esp_smartconfig.h>
#include <esp_wifi.h>

#include "TcpServer.h"

#include "Settings.h"

#define UDPPORT 2000

#define MAGICKEYNETWORK "589234"
#define APPID "APP"
#define DISCOVERCOMMAND       \
    MAGICKEYNETWORK "|" APPID \
                    "|"       \
                    "Discover"

WiFiUDP udp;

IPAddress broadcastIp;

char packetBuffer[256];

bool wifiConnected = false;

bool ConnectWifi(int timeoutSeconds = 5)
{
    if (WiFi.status() == WL_CONNECTED)
        return true;

    Serial.println("Starting WiFi..");
    WiFi.begin(settings.wificonfig.ssid, settings.wificonfig.password);

    int loopcounter = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        if (++loopcounter > timeoutSeconds)
            return false;
        delay(1000);
        Serial.println("Establishing connection to WiFi..");
    }

    wifiConnected = true;

    Serial.println("Connected to network");
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());

    return true;
}

bool smartConfigDone = false;
void smartConfigCallback(uint32_t st, void* result)
{
    smartconfig_status_t status = (smartconfig_status_t)st;
    if (status == SC_STATUS_GETTING_SSID_PSWD)
    {
    }
    else if (status == SC_STATUS_LINK)
    {
        wifi_sta_config_t* sta_conf = reinterpret_cast<wifi_sta_config_t*>(result);

        auto ssid = (char*)(sta_conf->ssid);
        auto pass = (char*)(sta_conf->password);

        strcpy(settings.wificonfig.ssid, ssid);
        strcpy(settings.wificonfig.password, pass);
        SaveSettings();

        Serial.println(ssid);
        Serial.println(pass);

        sta_conf->bssid_set = 0;
        esp_smartconfig_stop();

        delay(300);
        smartConfigDone = true;
    }
    else if (status == SC_STATUS_LINK_OVER)
    {
        WiFi.stopSmartConfig();
    }
}

void GetSmartConfigAsync()
{
    Serial.printf("\n Waiting for SmartConfig\n");

    WiFi.mode(WIFI_STA);

    delay(100);

    esp_smartconfig_start(reinterpret_cast<sc_callback_t>(&smartConfigCallback), 1);
}

void InitNetwork()
{
    broadcastIp = ~WiFi.subnetMask() | WiFi.gatewayIP();
    udp.begin(UDPPORT);

    InitTcpServer();

    delay(100);
}

bool startsWith(const char* pre, const char* str)
{
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void HandleUdpEvents()
{
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = udp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(udp.remotePort());

        // read the packet into packetBufffer
        int len = udp.read(packetBuffer, 256);
        if (len > 0)
        {
            packetBuffer[len] = 0;
        }
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        if (startsWith(DISCOVERCOMMAND, packetBuffer))
        {
            Serial.println("Recieved discover command.");
            Serial.println("Sending IP.\n");

            for (int i = 0; i < 2; i++)
            {
                udp.beginPacket(udp.remoteIP(), udp.remotePort());
                udp.printf("%s|%s|%s", MAGICKEYNETWORK, settings.controlSettings.netId,
                           WiFi.localIP().toString().c_str());
                udp.endPacket();
                delay(100);
            }
        }
    }
}

void InitOTA()
{
    ArduinoOTA.setHostname(settings.controlSettings.netId)
        .onStart([]() { Serial.println("Start updating "); })
        .onEnd([]() { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}
