#include <Arduino.h>
#include <LittleFS.h>

const int ledPin = 2; // Integrierter LED-Pin des ESP8266

void print_esp_infos()
{
  Serial.println("===============================");
  Serial.println("ESP8266 Infos:");

  Serial.print("CPU Frequenz (MHz): "); // Geschwindigkeit des Hauptprozessors
  Serial.println(ESP.getCpuFreqMHz());

  Serial.print("Flash Chip Größe (KB): "); // Gesamter Speicherplatz des Flash-Speichers
  Serial.println(ESP.getFlashChipSize() / 1024);

  Serial.print("Freier Arbeitsspeicher (KB): "); // Verfügbare Größe des Arbeitsspeichers
  Serial.println(ESP.getFreeHeap() / 1024);

  Serial.print("Programmgröße (KB): "); // Größe des hochgeladenen Programms
  Serial.println(ESP.getSketchSize() / 1024);

  Serial.print("Freier Speicher für Programme (KB): "); // Verfügbare Größe für zukünftige Programme
  Serial.println(ESP.getFreeSketchSpace() / 1024);

  if (!LittleFS.begin())
  {
    Serial.println("Fehler beim Initialisieren von LittleFS!");
  }
  else
  {
    FSInfo fs_info;
    if (LittleFS.info(fs_info))
    {
      Serial.print("Gesamter Dateisystem-Speicher (KB): "); // Gesamter Speicherplatz des LittleFS-Dateisystems
      Serial.println(fs_info.totalBytes / 1024);
      Serial.print("Belegter Dateisystem-Speicher (KB): "); // Belegter Speicherplatz im LittleFS-Dateisystem
      Serial.println(fs_info.usedBytes / 1024);
      Serial.print("Freier Dateisystem-Speicher (KB): "); // Verfügbare Größe im LittleFS-Dateisystem
      Serial.println((fs_info.totalBytes - fs_info.usedBytes) / 1024);
    }
    else
    {
      Serial.println("Fehler beim Abrufen von LittleFS-Informationen!");
    }
  }
  Serial.println("===============================");
}

void setup()
{
  Serial.begin(9600);
  delay(1000);

  pinMode(ledPin, OUTPUT);
  print_esp_infos();
}

void breathingEffect()
{
  // Atmungs- oder Fade-Effekt für die LED
  Serial.println("Fadein");
  for (int i = 255; i >= 0; i++)
  {
    analogWrite(ledPin, i);
    delay(20);
  }

  Serial.println("Fadeout");
  for (int i = 0; i <= 255; i++)
  {
    analogWrite(ledPin, i);
    delay(20);
  }
}

void loop()
{
  Serial.println("Setze die PWM-Frequenz auf 5 Hz");
  analogWriteFreq(5); // braucht ihr normalerweise nicht setzen, macht das nur für die demo
  breathingEffect();

  Serial.println("Setze die PWM-Frequenz auf 20 Hz");
  analogWriteFreq(20); // braucht ihr normalerweise nicht setzen, macht das nur für die demo
  breathingEffect();

  Serial.println("Setze die PWM-Frequenz auf 1000 Hz (Standardfrequenz)");
  analogWriteFreq(1000); // braucht ihr normalerweise nicht setzen, macht das nur für die demo
  breathingEffect();
}
