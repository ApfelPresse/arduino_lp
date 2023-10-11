#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include <DNSServer.h>

#define PIN_D1 5 // PWM_A
#define PIN_D2 4 // PWM_B
#define PIN_D3 0 // DA
#define PIN_D4 2 // DB

int PWMA = D1; // Right side
int PWMB = D2; // Left side
int DA = D3;   // Right reverse
int DB = D4;   // Left reverse

bool looping = false;

AsyncWebServer server(80);

IPAddress apIP(8, 8, 8, 8);
IPAddress netMsk(255, 255, 255, 0);

DNSServer dnsServer;

void config_ap_response()
{
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html").setCacheControl("max-age=30");
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
                          Serial.println(request->url());
                          if (request->url().endsWith(F(".js"))) {
                            Serial.println(F("js file"));                          
                          }
                          Serial.println("NOT FOUND " + request->url());                          
                          Serial.println(F("Redirect to AP Point"));
                          request->send(LittleFS, "/index.html", String(), false); });
}

void setup_ap()
{
  Serial.println(F("Starting AP!"));
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("robo");
  Serial.println(F("Gateway IP address: "));
  Serial.println(WiFi.softAPIP());
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", apIP);
}

void setup_littlefs()
{
  Serial.println(F("Configure Littlefs.."));
  if (!LittleFS.begin())
  {
    Serial.println(F("LittleFS beginn FAILED!"));
  }
}

void setup_webserver()
{
  Serial.println(F("Starting Webserver.."));
  server.begin();
  Serial.println(F("Configure AP Response.."));
  config_ap_response();
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  setup_littlefs();
  setup_ap();
  setup_webserver();

  Serial.println(F("Looping.."));
}

void loop()
{
  dnsServer.processNextRequest();
}