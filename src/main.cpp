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
#include <DallasTemperature.h>
#include <OneWire.h>

#ifndef MONITOR_BAUD
#define MONITOR_BAUD 9600
#endif

#ifndef HTTP_DOMAIN
#define HTTP_DOMAIN "TempSense-DEV"
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
#define SENSOR_TYPE "Temperature"
#endif

#ifndef ONE_WIRE_BUS
#define ONE_WIRE_BUS 4
#endif

#ifndef PLATFORM_NAME
#define PLATFORM_NAME "TempSense"
#endif

#ifndef PLATFORM_VERSION
#define PLATFORM_VERSION "1.0.0"
#endif

#define PLATFORM_BUILD_DATE __DATE__ ", " __TIME__
#define PLATFORM_DEVELOPER "Iordanis Kostelidis <iordkost@ihu.gr>"
#define PLATFORM_UNIVERSITY "International Hellenic University"
#define PLATFORM_DEPARTMENT "Department of Computer, Informatics and Telecommunications Engineering"
#define PLATFORM_ACADEMIC_PROGRAM "MSc in Robotics"

auto DEVICE_SERIAL_BAUD = MONITOR_BAUD;
auto DEVICE_HTTP_DOMAIN = HTTP_DOMAIN;
auto DEVICE_HTTP_PORT = HTTP_PORT;
auto WIFI_SSID_NAME = SSID_NAME;
auto WIFI_SSID_PASSWORD = SSID_PASSWORD;

ESP8266WebServer server(DEVICE_HTTP_PORT);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void printPlatformInfo();
void handleStatus();
void handleData();
void handleNotFound();
int32_t getTempByIndex(uint8_t deviceIndex);

void setup()
{
    Serial.begin(DEVICE_SERIAL_BAUD);
    printPlatformInfo();

    sensors.begin();

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

    if (MDNS.begin(DEVICE_HTTP_DOMAIN))
    {
        Serial.println("MDNS responder started");
        Serial.print("Hostname: ");
        Serial.println(WiFi.hostname());
    }

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

void printPlatformInfo()
{
    Serial.println(PLATFORM_NAME);
    Serial.print(" version\t\t");
    Serial.println(PLATFORM_VERSION);
    Serial.print(" build date\t\t");
    Serial.println(PLATFORM_BUILD_DATE);
    Serial.print(" developer\t\t");
    Serial.println(PLATFORM_DEVELOPER);
    Serial.print(" university\t\t");
    Serial.println(PLATFORM_UNIVERSITY);
    Serial.print(" department\t\t");
    Serial.println(PLATFORM_DEPARTMENT);
    Serial.print(" academic program\t");
    Serial.println(PLATFORM_ACADEMIC_PROGRAM);
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
    sensors.requestTemperatures();
    const int32_t temperatureRaw = getTempByIndex(0);
    const float temperatureC = sensors.getTempCByIndex(0);
    const float temperatureF = sensors.getTempFByIndex(0);

    JSONVar json;

    json["message"] = "OK";
    json["status"] = "200";

    json["raw"] = temperatureRaw;

    JSONVar processed;
    processed["celsius"] = temperatureC;
    processed["fahrenheit"] = temperatureF;
    json["processed"] = processed;

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

int32_t getTempByIndex(const uint8_t deviceIndex)
{
    DeviceAddress deviceAddress;
    if (!sensors.getAddress(deviceAddress, deviceIndex))
    {
        return DEVICE_DISCONNECTED_C;
    }
    return sensors.getTemp((uint8_t*)deviceAddress);
}
