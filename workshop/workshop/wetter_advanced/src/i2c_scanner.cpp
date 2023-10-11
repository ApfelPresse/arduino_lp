#include "i2c_scanner.h"

void scan_i2c()
{
    byte error, address;

    Serial.println("IC2 Scanning...");
    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("\t>I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");
        }
        else if (error == 4)
        {
            Serial.print("\t>Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    Serial.println("\t>Scan done\n");
}
