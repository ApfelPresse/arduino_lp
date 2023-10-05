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
const unsigned long sendInterval = 5000; // 5 Sekunden

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
                          //Serial.println(request->url());
                          //if (request->url().endsWith(F(".js"))) {
                          //  Serial.println(F("js file"));                          
                          //}
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

bool addSensorValuesToFile(const String &data, int maxLines = 20)
{
  File file = LittleFS.open("/data.txt", "a+");
  if (!file)
  {
    Serial.println("Failed to open file for appending.");
    return false;
  }

  file.println(data);
  file.close();

  Serial.println("Data appended: " + data);

  file = LittleFS.open("/data.txt", "r");
  if (!file)
  {
    Serial.println("Failed to open file for reading.");
    return false;
  }

  int lineCount = 0;
  while (file.available())
  {
    file.readStringUntil('\n');
    lineCount++;
  }
  file.close();

  Serial.println("Total lines in the file: " + String(lineCount));

  if (lineCount > maxLines)
  {
    String tempData = "";
    int currentLine = 0;

    file = LittleFS.open("/data.txt", "r");
    while (file.available())
    {
      currentLine++;
      String line = file.readStringUntil('\n');
      if (currentLine > 1)
      { // die erste Zeile löschen wir
        tempData += line + "\n";
      }
    }
    file.close();

    file = LittleFS.open("/data.txt", "w");
    if (!file)
    {
      Serial.println("Failed to open file for writing.");
      return false;
    }
    file.print(tempData);
    file.close();

    Serial.println("Removed the first line as the file exceeded the max line limit.");
  }

  return true;
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
  if (lightMeter.measurementReady())
  {
    float lux = lightMeter.readLightLevel();

    String data = "Light: " + String(lux) + " lx\n";
    data += "Temperature: " + String(bmp.readTemperature()) + " *C\n";
    data += "Pressure: " + String(bmp.readPressure()) + " Pa\n";
    data += "Altitude: " + String(bmp.readAltitude()) + " meters\n";
    data += "Pressure at sealevel (calculated): " + String(bmp.readSealevelPressure()) + " Pa\n";
    data += "Real altitude: " + String(bmp.readAltitude(101500)) + " meters\n";

    Serial.println(data);
    events.send(data.c_str(), "sensor_data", millis());
  }
}

void print_BH1750_light_sensor_data()
{
  if (lightMeter.measurementReady())
  {
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
  }
}

void print_BMP085_Barometric_sensor()
{
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(bmp.readSealevelPressure());
  Serial.println(" Pa");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(101500));
  Serial.println(" meters");

  Serial.println();
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