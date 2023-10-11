#include <Arduino.h>
#include <SPI.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <DNSServer.h>
#include <BH1750.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>


BH1750 lightMeter(0x23);
Adafruit_BMP085 bmp;

AsyncWebServer server(80);
AsyncEventSource events("/events");

DNSServer dnsServer;

unsigned long lastSentTime = 0;
const unsigned long sendInterval = 1000;

void setup_accesspoint()
{
  Serial.println(F("Starting AP!"));

  IPAddress apIP(8, 8, 8, 8);
  IPAddress netMsk(255, 255, 255, 0);

  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP("wetter");

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
  server.addHandler(&events);
  Serial.println(F("Configure AP Response.."));
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html").setCacheControl("max-age=30");
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
                          Serial.println("NOT FOUND " + request->url());                          
                          Serial.println(F("Redirect to AP Point"));
                          request->send(LittleFS, "/index.html", String(), false); });
}

void setup_BMP085_sensor()
{
  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
}

void setup_BH1750_light_sensor()
{
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE))
  {
    Serial.println(F("BH1750 Advanced begin"));
  }
  else
  {
    Serial.println(F("Error initialising BH1750"));
  }
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  Wire.begin();

  setup_littlefs();
  setup_accesspoint();
  setup_webserver();

  setup_BMP085_sensor();
  setup_BH1750_light_sensor();

  Serial.println(F("Looping.."));
}

void sendSensorDataToClients()
{
  float lux = lightMeter.readLightLevel();
  Serial.println("Lux (BH1750): " + String(lux));
  events.send(String(lux).c_str(), "light", millis());

  float temperature = bmp.readTemperature();
  Serial.println("Temperature (BMP085): " + String(temperature));
  events.send(String(temperature).c_str(), "temperature", millis());

  float pressure = bmp.readPressure();
  Serial.println("Pressure (BMP085): " + String(pressure));
  events.send(String(pressure).c_str(), "pressure", millis());

  float altitude = bmp.readAltitude();
  Serial.println("Altitude (BMP085): " + String(altitude));
  events.send(String(altitude).c_str(), "altitude", millis());

  float seaPressure = bmp.readSealevelPressure();
  Serial.println("Sea Pressure (BMP085): " + String(seaPressure));
  events.send(String(seaPressure).c_str(), "sea_pressure", millis());

  float realAltitude = bmp.readAltitude(101500);
  Serial.println("Real Altitude (BMP085 with standard pressure): " + String(realAltitude));
  events.send(String(realAltitude).c_str(), "real_altitude", millis());
}

void loop()
{
  dnsServer.processNextRequest();

  unsigned long currentMillis = millis();
  if (currentMillis - lastSentTime >= sendInterval)
  {
    sendSensorDataToClients();
    lastSentTime = currentMillis;
  }
}