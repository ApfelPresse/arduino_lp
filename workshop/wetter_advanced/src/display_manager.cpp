#include "display_manager.h"

// Definieren des logo_bmp Array, basierend auf Ihrem ursprünglichen Bitmap
const unsigned char PROGMEM DisplayManager::logo_bmp[] = {B00000000, B11000000,
                                                          B00000001, B11000000,
                                                          B00000001, B11000000,
                                                          B00000011, B11100000,
                                                          B11110011, B11100000,
                                                          B11111110, B11111000,
                                                          B01111110, B11111111,
                                                          B00110011, B10011111,
                                                          B00011111, B11111100,
                                                          B00001101, B01110000,
                                                          B00011011, B10100000,
                                                          B00111111, B11100000,
                                                          B00111111, B11110000,
                                                          B01111100, B11110000,
                                                          B01110000, B01110000,
                                                          B00000000, B00110000};

DisplayManager::DisplayManager() : currentTemperatureIndex(0), maxTemperature(-999), minTemperature(999), display(SCREEN_WIDTH, SCREEN_HEIGHT)
{
    for (int i = 0; i < MAX_TEMPERATURE_POINTS; i++)
    {
        temperatureData[i] = 0.0f; // oder einen anderen Standardwert, wie NAN
    }
}

void DisplayManager::setup()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("[❌] SSD1306 allocation failed"));
        return;
    }
    display.clearDisplay();

    for (int i = 0; i < LOGO_COUNT; i++)
    {
        logos[i].x = random(SCREEN_WIDTH - 16);
        logos[i].y = 0 - (sizeof(logo_bmp) / 2) - i * 18;
        logos[i].state = LOGO_FALLING;
        logos[i].explosionDuration = 0;
    }
}

void DisplayManager::demo_logo()
{
    display.clearDisplay();

    for (int i = 0; i < LOGO_COUNT; i++)
    {
        if (logos[i].state == LOGO_FALLING)
        {
            logos[i].y += 1;

            // Gelegentlich "explodiert" ein Logo
            if (random(500) < 2)
            {
                logos[i].state = EXPLODING;
                logos[i].explosionDuration = 10;
            }

            // Wenn das Logo den unteren Rand erreicht, setzen wir es zurück nach oben
            if (logos[i].y > SCREEN_HEIGHT)
            {
                logos[i].y = 0 - (sizeof(logo_bmp) / 2);
                logos[i].x = random(SCREEN_WIDTH - 16);
                logos[i].state = LOGO_FALLING;
            }
        }
        else if (logos[i].state == EXPLODING)
        {
            // Zeichne kleine Zacken an der Stelle des Logos während der Explosion
            display.drawLine(logos[i].x, logos[i].y + 8, logos[i].x - 5, logos[i].y + 8, 1);
            display.drawLine(logos[i].x + 16, logos[i].y + 8, logos[i].x + 21, logos[i].y + 8, 1);

            logos[i].explosionDuration--;
            // Nach einer kurzen Explosionsdauer wird das Logo zurückgesetzt
            if (logos[i].explosionDuration <= 0)
            {
                logos[i].y = 0 - (sizeof(logo_bmp) / 2);
                logos[i].x = random(SCREEN_WIDTH - 16);
                logos[i].state = LOGO_FALLING;
            }
        }

        // Wenn das Logo nicht in einem "EXPLODING" Zustand ist, zeichnen wir es
        if (logos[i].state != EXPLODING)
        {
            display.drawBitmap(logos[i].x, logos[i].y, logo_bmp, 16, 16, 1);
        }
    }

    display.display();
}

void DisplayManager::addTempData(float temperature)
{
    // Daten aktualisieren
    temperatureData[currentTemperatureIndex] = temperature;

    // Skalierung für das Diagramm aktualisieren
    if (temperature > maxTemperature)
        maxTemperature = temperature;
    if (temperature < minTemperature)
        minTemperature = temperature;

    // Index aktualisieren
    currentTemperatureIndex = (currentTemperatureIndex + 1) % MAX_TEMPERATURE_POINTS;
}

void DisplayManager::drawTempChart()
{
    const int xOffset = 20; // Puffer für die Y-Achse
    const int yOffset = 10; // Puffer für den oberen und unteren Rand

    // Berechnen Sie die Skalierung für die X-Achse, basierend auf verfügbarem Platz und MAX_TEMPERATURE_POINTS
    float scaleX = (float)(SCREEN_WIDTH - xOffset) / MAX_TEMPERATURE_POINTS;

    display.clearDisplay();

    // Zeichne Y-Achse
    display.drawLine(xOffset, yOffset, xOffset, SCREEN_HEIGHT - yOffset, WHITE);

    // Beschriftung für maximale und minimale Temperaturen
    display.setTextSize(1); // Größe 1 für kleine Schrift
    display.setTextColor(WHITE);
    display.setCursor(0, 0);                 // oben links
    display.print(maxTemperature, 1);        // 1 Dezimalstelle
    display.setCursor(0, SCREEN_HEIGHT - 8); // unten links, 8 Pixel Platz für den Text
    display.print(minTemperature, 1);        // 1 Dezimalstelle

    // Skalieren und zeichnen der Datenpunkte
    for (int i = 1; i < MAX_TEMPERATURE_POINTS; i++)
    {
        int prevX = xOffset + (i - 1) * scaleX;
        int currX = xOffset + i * scaleX;

        int prevY, currY;
        long tempPrev = temperatureData[(currentTemperatureIndex + i - 1) % MAX_TEMPERATURE_POINTS];
        long tempCurr = temperatureData[(currentTemperatureIndex + i) % MAX_TEMPERATURE_POINTS];
        tempPrev = constrain(tempPrev, minTemperature, maxTemperature);
        tempCurr = constrain(tempCurr, minTemperature, maxTemperature);

        if (minTemperature != maxTemperature)
        {
            prevY = map(tempPrev, minTemperature, maxTemperature, SCREEN_HEIGHT - yOffset, yOffset);
            currY = map(tempCurr, minTemperature, maxTemperature, SCREEN_HEIGHT - yOffset, yOffset);
        }
        else
        {
            // Falls minTemperature gleich maxTemperature ist, setzen Sie beide Y-Werte auf einen Standardwert, z.B. die Bildschirmmitte
            prevY = currY = SCREEN_HEIGHT / 2;
        }
       
        
        display.drawLine(prevX, prevY, currX, currY, WHITE);
    }
    display.display();
}
