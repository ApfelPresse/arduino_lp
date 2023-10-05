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

void stop()
{
  digitalWrite(PWMA, LOW);
  digitalWrite(DA, LOW);

  digitalWrite(PWMB, LOW);
  digitalWrite(DB, LOW);
}

void move_forward()
{
  analogWrite(PWMA, 255);
  digitalWrite(DA, HIGH);

  analogWrite(PWMB, 255);
  digitalWrite(DB, HIGH);
}

void move_left()
{
  analogWrite(PWMA, 255);
  digitalWrite(DA, LOW);

  analogWrite(PWMB, 255);
  digitalWrite(DB, HIGH);
}

void move_right()
{
  analogWrite(PWMA, 255);
  digitalWrite(DA, HIGH);

  analogWrite(PWMB, 255);
  digitalWrite(DB, LOW);
}

void move_backward()
{
  analogWrite(PWMA, 255);
  digitalWrite(DA, LOW);

  analogWrite(PWMB, 255);
  digitalWrite(DB, LOW);
}

int read_left_sensor()
{
  return digitalRead(D7);
}

int read_right_sensor()
{
  return digitalRead(D6);
}

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

  pinMode(PWMA, OUTPUT); // how fast should the motor turn
  pinMode(PWMB, OUTPUT);
  pinMode(DA, OUTPUT); // is this pin enbaled the motor will spin
  pinMode(DB, OUTPUT);
  pinMode(D7, INPUT); // we want to read the sensor from pin D7
  pinMode(D6, INPUT);

  setup_littlefs();
  setup_ap();
  setup_webserver();

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
                looping = true;
                request->send(200, "text/plain"); });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
                looping = false;
                request->send(200, "text/plain"); });


  Serial.println(F("Looping.."));
}

void loop()
{
  dnsServer.processNextRequest();
  if (looping) {
    if (read_left_sensor() == 1) {
      Serial.println(F("Turn left?")); 
    }
    if (read_right_sensor() == 1) {
      Serial.println(F("Turn right?")); 
    }
    move_left();
    delay(1000);
    move_right();
    delay(1000);    
  } else {
    stop();
  }
}