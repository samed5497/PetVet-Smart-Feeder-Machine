#include <core/chooser.h>

#ifdef TEST
#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(3, 4, NEO_GRB + NEO_KHZ800);

void setup()
{
#if defined(__AVR_ATtiny85__) && (F_CPU == 26000000)
    clock_prescale_set(clock_div_1);
#endif
    Serial.begin(115200);
    pinMode(12, INPUT);
    pinMode(16, OUTPUT);
    pixels.begin();
}
void loop()
{

    Serial.println("manyetik  alan");
    Serial.println(digitalRead(12));
    digitalWrite(16, 1);
    pixels.setPixelColor(0, 255, 0, 0);
    pixels.show();
    delay(1 * 1000);
    pixels.setPixelColor(1, 0, 255, 46);
    pixels.show();
    delay(1 * 1000);
    pixels.setPixelColor(2, 152, 0, 255);
    pixels.show();
    delay(1 * 1000);
    digitalWrite(16, 0);
    delay(1 * 1000);
}
#endif
