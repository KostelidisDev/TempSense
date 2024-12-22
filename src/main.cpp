/*  MIT License

    Copyright (c) 2024 Kostelidis.dev

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Arduino_JSON.h>

#ifndef MONITOR_BAUD
#define MONITOR_BAUD 9600
#endif

#ifndef HTTP_DOMAIN
#define HTTP_DOMAIN "NodeSense-DEV"
#endif

#ifndef HTTP_PORT
#define HTTP_PORT 80
#endif

#ifndef SSID_NAME
#define SSID_NAME "WiFi"
#endif

#ifndef SSID_PASSWORD
#define SSID_PASSWORD "WiFi@PASSWORD"
#endif

#ifndef SENSOR_TYPE
#define SENSOR_TYPE "Template"
#endif

auto DEVICE_SERIAL_BAUD = MONITOR_BAUD;
auto DEVICE_HTTP_DOMAIN = HTTP_DOMAIN;
auto DEVICE_HTTP_PORT = HTTP_PORT;
auto WIFI_SSID_NAME = SSID_NAME;
auto WIFI_SSID_PASSWORD = SSID_PASSWORD;

ESP8266WebServer server(DEVICE_HTTP_PORT);

void handleStatus();
void handleData();
void handleNotFound();

void setup()
{
    Serial.begin(DEVICE_SERIAL_BAUD);

    Serial.println("");
    Serial.println(WIFI_SSID_NAME);
    Serial.println(WIFI_SSID_PASSWORD);
    Serial.println("");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID_NAME, WIFI_SSID_PASSWORD);
    WiFi.hostname(DEVICE_HTTP_DOMAIN);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID_NAME);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(DEVICE_HTTP_DOMAIN)) { Serial.println("MDNS responder started"); }

    server.on("/", handleStatus);
    server.on("/status", handleStatus);
    server.on("/data", handleData);
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop()
{
    server.handleClient();
    MDNS.update();
}

void handleStatus()
{
    JSONVar json;

    json["message"] = "OK";
    json["status"] = "200";

    json["ip"] = WiFi.localIP().toString();
    json["hostname"] = DEVICE_HTTP_DOMAIN;
    json["sensor"] = SENSOR_TYPE;
    json["uptime"] = millis() / 1000;

    const String jsonResponse = JSON.stringify(json);

    server.send(200, "application/json", jsonResponse);
}

void handleData()
{
    JSONVar json;

    json["message"] = "OK";
    json["status"] = "200";

    json["raw"] = nullptr;

    json["processed"] = nullptr;

    const String jsonResponse = JSON.stringify(json);

    server.send(200, "application/json", jsonResponse);
}

void handleNotFound()
{
    JSONVar json;

    json["message"] = "Not found";
    json["status"] = "404";

    const String jsonResponse = JSON.stringify(json);

    server.send(200, "application/json", jsonResponse);
}
