#include <core/variables.h>

void Feed_Time_Tracking_Fonction()
{
    //
    for (int i = 1; i <= PROGRAM_SAYISI; i++)
    {
        if (feed_mode == false &&
            last_feed_minute != currentMinute &&
            feed_time_hour[i] == currentHour &&
            feed_time_minute[i] == currentMinute &&
            status[i] == true)
        {
            Serial.print("[INFO]: Feeding Time. Program: ");
            Serial.println(i);
            runing_program = i;
            feed_mode = true;
            last_feed_minute = currentMinute;
            break; // Bir kez eşleşme bulunursa döngüyü sonlandır
        }
    }
}

/////////////////////   MOTOR FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void motoru_baslat(bool ters_yon)
{
    digitalWrite(m1_en_pin, HIGH);
    if (ters_yon)
    {
        digitalWrite(m1_in1_pin, HIGH);
        digitalWrite(m1_in2_pin, LOW);
    }
    else
    {
        digitalWrite(m1_in1_pin, LOW);
        digitalWrite(m1_in2_pin, HIGH);
    }
}

void motoru_durdur()
{
    digitalWrite(m1_en_pin, LOW);
}

/////////////////////   MOTOR KONTROL FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void donme_kontrol()
{
    if (motor_lap_count < -1)
    {
        motor_lap_count = -1;
    }
    if (digitalRead(switch_pin) == HIGH and switch_durum == false) // 150ms de bir switch durumu degiştikçe (tıklandıkça) degeri degiştir.
    {
        son_tiklama_zamani = millis();
        motor_lap_count++;
        Serial.print("TUR     : ");
        Serial.println(motor_lap_count);
        Serial.println();

        // Serial.print("son_tik :");
        // Serial.println(son_tiklama_zamani);
        switch_durum = true;
    }

    if (millis() - son_tiklama_zamani > 1000) // 1 saniye içinde artış olmazsa motoru ters yönde çalıştır
    {
        Serial.println("[HATA]: Sıkışma Algılandı - 1");
        motoru_durdur();
        motoru_baslat(false);

        if (digitalRead(switch_pin) == LOW)
        {
            while (digitalRead(switch_pin) == LOW)
            {

                // buraya da bu durumun 2.5 saniyeden fazla surmesı durumunda error kodu ekle:
            }
        }
        if (digitalRead(switch_pin) == HIGH)
        {
            while (digitalRead(switch_pin) == HIGH)
            {
                // buraya da bu durumun 2.5 saniyeden fazla surmesı durumunda error kodu ekle:
            }
        }
        son_tiklama_zamani = millis();
        motor_lap_count--;
        motoru_durdur();

        motoru_baslat(true);
    }

    if (millis() - son_tiklama_zamani > 150 and switch_durum == true and digitalRead(switch_pin) == LOW) // swich tıklandıgında basılı kalmaması için sayaç.
    {
        switch_durum = false;
    }
}

/////////////////////   NEOPIXEL FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, c);
        strip.show();
        vTaskDelay(wait);
    }
}

/////////////////////   Feeder Sound FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void Feeder_Sound()
{
    tone(buzzer_pin, 500); // 1000 Hz frekansta ses üret
    vTaskDelay(50);        // 250 milisaniye bekle
    noTone(buzzer_pin);    // Sesi kapat
    vTaskDelay(50);        // 250 milisaniye bekle
    tone(buzzer_pin, 500); // 1000 Hz frekansta ses üret
    vTaskDelay(50);        // 250 milisaniye bekle
    noTone(buzzer_pin);    // Sesi kapat
    vTaskDelay(50);        // 250 milisaniye bekle
    tone(buzzer_pin, 500); // 1000 Hz frekansta ses üret
    vTaskDelay(50);        // 250 milisaniye bekle
    noTone(buzzer_pin);    // Sesi kapat
}

void Water_Feeder_Sound()
{
    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
    vTaskDelay(100);        // 250 milisaniye bekle
    noTone(buzzer_pin);     // Sesi kapat
    vTaskDelay(100);        // 250 milisaniye bekle
    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
    vTaskDelay(100);        // 250 milisaniye bekle
    noTone(buzzer_pin);     // Sesi kapat
}

/////////////////////   DAHİLİ EEPROM FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

// EEPROM'a güvenli bir şekilde yazmak için fonksiyon
void EEPROM_Write(int address, int value)
{
    EEPROM.writeInt(address, value);
    EEPROM.commit();
}

// EEPROM'dan veri okumak için fonksiyon
int EEPROM_Read(int address)
{
    return EEPROM.readInt(address);
}

/////////////////////  RTC EEPROM FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void writeEEPROM(int address, int data)
{
    Wire.beginTransmission(EEPROM_ADDRESS); // 24C32'nin adresi
    Wire.write((byte)(address >> 8));       // Yüksek adres byte
    Wire.write((byte)(address & 0xFF));     // Düşük adres byte
    Wire.write((byte)data);                 // Veri
    Wire.endTransmission();
}

// EEPROM'dan veri okuma fonksiyonu
int readEEPROM(int address)
{
    int data = 0;                           // Okunan veri
    Wire.beginTransmission(EEPROM_ADDRESS); // 24C32'nin adresi
    Wire.write((byte)(address >> 8));       // Yüksek adres byte
    Wire.write((byte)(address & 0xFF));     // Düşük adres byte
    Wire.endTransmission(false);

    Wire.requestFrom(EEPROM_ADDRESS, 1); // 1 byte oku
    if (Wire.available())
    {
        data = Wire.read(); // Okunan veriyi al
    }
    return data;
}

void Epprom_Update_From_PC()
{
    Serial.println("[Info]: CİHAZIN İLK SAAT KURUKUMU YAPILDI.");

    // Yıl, ay ve günü EEPROM'a yazma
    writeEEPROM(60, 24); // Yılı iki haneli olarak kaydet (örneğin 2024 -> 24)
    vTaskDelay(5);
    writeEEPROM(50, 8); // Ayı kaydet
    vTaskDelay(5);
    writeEEPROM(40, 17); // Günü kaydet
    vTaskDelay(5);
    writeEEPROM(70, 6);
    vTaskDelay(5);

    // Saat, dakika ve saniyeyi EEPROM'a yazma
    writeEEPROM(10, 0); // Saati kaydet
    vTaskDelay(5);
    writeEEPROM(20, 0); // Dakikayı kaydet
    vTaskDelay(5);
    writeEEPROM(30, 15); // Saniyeyi kaydet
    vTaskDelay(5);
}

// RTC değerlerini EEPROM'a yazma işlemi
void writeRTCtoEEPROM()
{
    if (currentDayofWeek != readEEPROM(70))
    {
        writeEEPROM(70, rtc.dayOfWeek());
        vTaskDelay(5);
        writeEEPROM(40, rtc.day());
        vTaskDelay(5);
        writeEEPROM(50, rtc.month());
        vTaskDelay(5);
        writeEEPROM(60, rtc.year());
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Gün Güncellendi: %d\n", currentDayofWeek);
    }
    if (currentHour != readEEPROM(10))
    {
        writeEEPROM(10, rtc.hour());
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Saat Güncellendi: %d\n", currentHour);
        Local_Time_Report = true;
    }
    if (currentMinute != readEEPROM(20))
    {
        writeEEPROM(20, rtc.minute());
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Dakika Güncellendi: %d\n", currentMinute);
        // device_server_report = true;
    }
    if (currentSecond != readEEPROM(30))
    {
        writeEEPROM(30, rtc.second());
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Saniye Güncellendi: %d\n", currentSecond);
    }
    Serial.println();
    /*
    writeEEPROM(10, currentHour);
    vTaskDelay (5); // Yazma işlemi için biraz daha uzun bir gecikme ekleyin
    writeEEPROM(20, currentMinute);
    vTaskDelay (5);
    writeEEPROM(30, currentSecond);
    vTaskDelay (5);
    writeEEPROM(40, currentDay);
    vTaskDelay (5);
    writeEEPROM(50, currentMonth);
    vTaskDelay (5);
    writeEEPROM(60, currentYear);
    vTaskDelay (5);
    writeEEPROM(70, currentDayofWeek);
    vTaskDelay (5);
*/
}

/////////////////////   RTC BASLATMA FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void ZamanGuncelleYazdir()
{
    rtc.refresh();

    currentDayofWeek = rtc.dayOfWeek();
    currentYear = rtc.year();
    currentMonth = rtc.month();
    currentDay = rtc.day();
    currentSecond = rtc.second();
    currentMinute = rtc.minute();
    currentHour = rtc.hour();

    /*
        Serial.print("Saat: ");
        Serial.print(rtc.hour());
        Serial.print(":");
        Serial.print(rtc.minute());
        Serial.print(":");
        Serial.print(rtc.second());

        Serial.print(" *** Tarih: ");
        Serial.print(rtc.day());
        Serial.print("/");
        Serial.print(rtc.month());
        Serial.print("/");
        Serial.print(rtc.year());
        Serial.print(" *** ");
        Serial.print(rtc.dayOfWeek());
        Serial.println(".gün");
    */

    if (rtcModel == URTCLIB_MODEL_DS1307 || rtcModel == URTCLIB_MODEL_DS3232)
    {
        // Serial.print("~ SRAM position   : ");
        // Serial.print(position);
        // Serial.print(" / value: ");
        // Serial.println(rtc.ramRead(position), HEX);
        position++;
    }
    Serial.flush();
}

/////////////////////   removeErrorCode FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

// Belirli bir hata kodunu silen fonksiyon
void removeErrorCode(String errorCode)
{
    int index = hata_kodu.indexOf(errorCode);

    if (index != -1)
    {
        int length = errorCode.length();

        // Hata kodunu sil ve varsa önündeki veya arkasındaki virgülü kaldır
        if (index + length < hata_kodu.length() && hata_kodu.charAt(index + length) == ',')
        {
            // Hata kodunun sonunda virgül varsa
            hata_kodu.remove(index, length + 1);
        }
        else if (index > 0 && hata_kodu.charAt(index - 1) == ',')
        {
            // Hata kodunun başında virgül varsa
            hata_kodu.remove(index - 1, length + 1);
        }
        else
        {
            // Sadece hata kodunu sil
            hata_kodu.remove(index, length);
        }
    }
}

/////////////////////   SeriPortGelenKontrol FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void SeriPortGelenKontrol()
{
    if (Serial.available() > 0)
    {                                                          // Seri portta veri varsa
        String receivedMessage = Serial.readStringUntil('\n'); // Satır sonuna kadar olan mesajı oku
                                                               // Serial.println("Received message: " + receivedMessage); // Gelen mesajı yazdır

        // Gelen mesajdaki ID'yi kontrol et
        if (receivedMessage.startsWith("i") or receivedMessage.startsWith("İ"))
        {
            String gelenmesaj = receivedMessage.substring(1);
            float sonuc = gelenmesaj.toInt(); // Rakamsal değere dönüştür
            if (sonuc > 0 or sonuc <= 10)
            {
            }
            else
            {
            }
        }
    }
}

void Ready_to_Send_Datas_1()
{
    if (manuel_water_status_report) // manuel su alma komutunu aldıktan sonra geri bildirim olarak komutu tekrar false yapıyor.
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/programs/manuel/manuelwater");
        if (Firebase.RTDB.setBool(&fbdo, VeriYolu, false))
        {
            manuel_water_status_report = false;
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(5);
    }

    ///////////////////////////////////////////////////////////////////////

    if (manuel_feeder_status_report) // manuel su alma komutunu aldıktan sonra geri bildirim olarak komutu tekrar false yapıyor.
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/programs/manuel/manuelfeed");
        if (Firebase.RTDB.setBool(&fbdo, VeriYolu, false))
        {
            manuel_feeder_status_report = false;
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1);
    }
}

void Ready_to_Send_Datas_2()
{
    if (Local_Time_Report)
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/device/lastreport");

        // Değişkenleri String'e dönüştür
        String yearStr = String(currentYear);
        String monthStr = String(currentMonth); // 'currentMonth' yerine 'currentMonth' kullanıldı.
        String dayStr = String(currentDay);     // 'currentDay' değil 'currentDay' olarak düzenlendi.
        String hourStr = String(currentHour);
        String minuteStr = String(currentMinute);
        String secondStr = String(currentSecond);

        // Tarih ve saati uygun formatta birleştir
        String Saat = dayStr + "/" + monthStr + "/" + yearStr + " " +
                      hourStr + ":" + minuteStr + ":" + secondStr;

        if (Firebase.RTDB.setString(&fbdo, VeriYolu, Saat))
        {
            Local_Time_Report = false;
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1); // Boşluk kaldırıldı
    }

    ///////////////////////////////////////////////////////////////////////

    if (alarm_status)
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/device/alarmcode");
        if (Firebase.RTDB.setString(&fbdo, VeriYolu, hata_kodu))
        {
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1);
    }

    // EK - BURAYI DÜZELT
    /*
       if (last_manuel_mod_feed_time != SERVER_last_manuel_mod_feed_time)
       {
           VeriYolu.clear();
           VeriYolu.concat(uid);
           VeriYolu.concat("/device/lastmanuelfeedtime");

           if (Firebase.RTDB.setString(&fbdo, VeriYolu, last_manuel_mod_feed_time))
           {
           }
           else
           {
               Serial.print("[ERROR]: Connection FAILED. ");
               Serial.println("REASON: " + fbdo.errorReason());
           }
           vTaskDelay (1);
       }
 */

    if (Food_Level != SERVER_Food_Level)
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/device/foodlevel");
        if (Firebase.RTDB.setInt(&fbdo, VeriYolu, Food_Level))
        {
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1);
    }

    ///////////////////////////////////////////////////////////////////////

    if (Water_Level != SERVER_Water_Level)
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/device/waterlevel");
        if (Firebase.RTDB.setInt(&fbdo, VeriYolu, Water_Level))
        {
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1);
    }

    ///////////////////////////////////////////////////////////////////////

    if (batterylevel != SERVER_batterylevel)
    {
        VeriYolu.clear();
        VeriYolu.concat(uid);
        VeriYolu.concat("/device/batterylevel");
        if (Firebase.RTDB.setInt(&fbdo, VeriYolu, batterylevel))
        {
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }
        vTaskDelay(1);
    }

    ///////////////////////////////////////////////////////////////////////
}