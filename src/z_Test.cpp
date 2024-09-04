#include <core/chooser.h>

#ifdef TEST
#include "Arduino.h"
#include "uRTCLib.h"
#include <Wire.h> // I2C kütüphanesini dahil edin

uRTCLib rtc;

void setup()
{
    pinMode(33, OUTPUT);
    digitalWrite(33, HIGH);
    Serial.begin(115200);
    Wire.begin();                        // I2C hattını başlat
    rtc.set_rtc_address(0x68);           // RTC adresini ayarla
    rtc.set_model(URTCLIB_MODEL_DS1307); // RTC modelini ayarla
    rtc.set_12hour_mode(false);          // 24 saatlik formatı kullan

    // RTC'nin pil ile çalıştığından emin ol
    if (rtc.enableBattery())
    {
        Serial.println("Pil etkinleştirildi.");
    }
    else
    {
        Serial.println("Pil etkinleştirilemedi.");
    }

    // RTC'yi yalnızca bir kez ayarla, ardından bu kodu yorum satırı haline getir
    delay(2000);
    rtc.refresh();
    Serial.print("HAFIZADAKİ RTC Tarih ve Saat: ");
    Serial.print(rtc.year());
    Serial.print('/');
    Serial.print(rtc.month());
    Serial.print('/');
    Serial.print(rtc.day());
    Serial.print(' ');
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.print(rtc.second());
    Serial.println();
    
    if (rtc.year() == 0)
    { // Eğer RTC sıfırdan başlıyorsa, saati ayarla
        Serial.println("RTC sıfırlandı. Zamanı ayarlıyorum...");
        rtc.set(0, 30, 16, 4, 15, 8, 24); // Saati burada ayarlayın (second, minute, hour, dayOfWeek, dayOfMonth, month, year)
        Serial.println("Zaman ayarlandı.");
    }
    else
    {
        Serial.println("RTC mevcut zamanı alıyor.");
    }
}

void loop()
{
    rtc.refresh();
    Serial.print("RTC Tarih ve Saat: ");
    Serial.print(rtc.year());
    Serial.print('/');
    Serial.print(rtc.month());
    Serial.print('/');
    Serial.print(rtc.day());
    Serial.print(' ');
    Serial.print(rtc.hour());
    Serial.print(':');
    Serial.print(rtc.minute());
    Serial.print(':');
    Serial.print(rtc.second());
    Serial.println();

    delay(1000); // Her saniye RTC'yi güncelle
}

#endif
