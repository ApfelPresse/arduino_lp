#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#define MAX_TEMPERATURE_POINTS 10

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class DisplayManager {
public:
    DisplayManager();
    void setup();
    void demo_logo();

    void addTempData(float temperature);
    void drawTempChart();

private:
    static const int LOGO_COUNT = 5;
    static const int SCREEN_WIDTH = 128;
    static const int SCREEN_HEIGHT = 64;

    float temperatureData[MAX_TEMPERATURE_POINTS]; // Speichert die letzten Temperaturdatenpunkte
    int currentTemperatureIndex; // Aktueller Index im Temperaturdatenarray
    float maxTemperature; // Zum Skalieren des Diagramms
    float minTemperature;

    enum LogoState {
        LOGO_FALLING,
        STATIC,
        EXPLODING
    };

    struct LogoPosition {
        int x;
        int y;
        LogoState state;
        int explosionDuration;
    };

    LogoPosition logos[LOGO_COUNT];

    Adafruit_SSD1306 display;

    static const unsigned char PROGMEM logo_bmp[];
};

#endif
