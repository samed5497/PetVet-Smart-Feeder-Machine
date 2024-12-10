#include <core/variables.h>

/////////////////////   OTO GÜNCELLEME FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void RESUMEALLTASKS()
{
    if (TaskTimeControlHandle != NULL)
    {
        vTaskResume(TaskTimeControlHandle);
    }
    if (TaskFeederHandle != NULL)
    {
        vTaskResume(TaskFeederHandle);
    }
    if (TaskSoundControlHandle != NULL)
    {
        vTaskResume(TaskSoundControlHandle);
    }
    if (TaskServerConnectionHandle != NULL)
    {
        vTaskResume(TaskServerConnectionHandle);
    }
    if (TaskButtonControlHandle != NULL)
    {
        vTaskResume(TaskButtonControlHandle);
    }
    if (TaskFeederFlowControlHandle != NULL)
    {
        vTaskResume(TaskFeederFlowControlHandle);
    }
    if (TaskAlarmControlHandle != NULL)
    {
        vTaskResume(TaskAlarmControlHandle);
    }
    if (TaskLightControlHandle != NULL)
    {
        vTaskResume(TaskLightControlHandle);
    }
    if (TaskWaterFeederHandle != NULL)
    {
        vTaskResume(TaskWaterFeederHandle);
    }
    if (TaskMicControlHandle != NULL)
    {
        vTaskResume(TaskMicControlHandle);
    }
    if (TaskWaterLevelControlHandle != NULL)
    {
        vTaskResume(TaskWaterLevelControlHandle);
    }
    if (TaskFoodLevelControlHandle != NULL)
    {
        vTaskResume(TaskFoodLevelControlHandle);
    }
    if (TaskSerialPortReportHandle != NULL)
    {
        vTaskResume(TaskSerialPortReportHandle);
    }
    if (TaskBatteryControlHandle != NULL)
    {
        vTaskResume(TaskBatteryControlHandle);
    }
    Serial.println("Tüm görevler yeniden başlatıldı.");
}

void STOPALLTASKSFORUPDATE()
{
    if (TaskTimeControlHandle != NULL)
    {
        vTaskSuspend(TaskTimeControlHandle);
    }
    if (TaskFeederHandle != NULL)
    {
        vTaskSuspend(TaskFeederHandle);
    }
    if (TaskSoundControlHandle != NULL)
    {
        vTaskSuspend(TaskSoundControlHandle);
    }
    if (TaskServerConnectionHandle != NULL)
    {
        vTaskSuspend(TaskServerConnectionHandle);
    }
    if (TaskButtonControlHandle != NULL)
    {
        vTaskSuspend(TaskButtonControlHandle);
    }
    if (TaskFeederFlowControlHandle != NULL)
    {
        vTaskSuspend(TaskFeederFlowControlHandle);
    }
    if (TaskAlarmControlHandle != NULL)
    {
        vTaskSuspend(TaskAlarmControlHandle);
    }
    if (TaskLightControlHandle != NULL)
    {
        vTaskSuspend(TaskLightControlHandle);
    }
    if (TaskWaterFeederHandle != NULL)
    {
        vTaskSuspend(TaskWaterFeederHandle);
    }
    if (TaskMicControlHandle != NULL)
    {
        vTaskSuspend(TaskMicControlHandle);
    }
    if (TaskWaterLevelControlHandle != NULL)
    {
        vTaskSuspend(TaskWaterLevelControlHandle);
    }
    if (TaskFoodLevelControlHandle != NULL)
    {
        vTaskSuspend(TaskFoodLevelControlHandle);
    }
    if (TaskSerialPortReportHandle != NULL)
    {
        vTaskSuspend(TaskSerialPortReportHandle);
    }
    if (TaskBatteryControlHandle != NULL)
    {
        vTaskSuspend(TaskBatteryControlHandle);
    }
    Serial.println("Tüm görevler güncelleme için durduruldu.");
}

void DELETEALLTASKSFORUPDATE()
{
    vTaskSuspend(TaskTimeControlHandle);
    vTaskSuspend(TaskFeederHandle);
    vTaskSuspend(TaskSoundControlHandle);
    vTaskSuspend(TaskServerConnectionHandle);
    vTaskSuspend(TaskButtonControlHandle);
    vTaskSuspend(TaskFeederFlowControlHandle);
    vTaskSuspend(TaskAlarmControlHandle);
    vTaskSuspend(TaskLightControlHandle);
    vTaskSuspend(TaskWaterFeederHandle);
    vTaskSuspend(TaskMicControlHandle);
    vTaskSuspend(TaskWaterLevelControlHandle);
    vTaskSuspend(TaskFoodLevelControlHandle);
    vTaskSuspend(TaskSerialPortReportHandle);
    vTaskSuspend(TaskBatteryControlHandle);
}

void performOTAUpdate(String url) // OTA güncelleme işlemi
{
    Serial.println("[Info]: Firmware indiriliyor: " + url);

    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
        WiFiClient *client = http.getStreamPtr();
        size_t contentLength = http.getSize();

        if (contentLength > 0 && Update.begin(contentLength))
        {
            size_t written = 0;
            int lastPercentage = 0; // Son yazdırılan yüzde değeri

            // Diğer görevleri durdur
            updating = true;
            STOPALLTASKSFORUPDATE();

            while (written < contentLength)
            {
                uint8_t buffer[128];
                size_t bytesRead = client->readBytes(buffer, sizeof(buffer));
                written += Update.write(buffer, bytesRead);
                delay(1); // RTOS'a zaman tanımak için

                // Yüzde hesaplama ve seri porta yazdırma
                int percentage = (written * 100) / contentLength;
                if (percentage - lastPercentage >= 5) // 5'er 5'er güncelle
                {
                    Serial.printf("[Info]: Yükleme durumu: %d%%\n", percentage);
                    lastPercentage = percentage;
                }
            }

            if (Update.end(true))
            {
                Serial.println("[Info]: OTA başarılı! Cihaz yeniden başlatılıyor...");
                ESP.restart();
            }
            else
            {
                Serial.printf("[ERROR]: OTA doğrulama hatası: %s\n", Update.errorString());
            }
            /*
                        // Diğer görevleri devam ettir
                        updating = false;
                        RESUMEALLTASKS();
                        */
        }
        else
        {
            Serial.println("[ERROR]: Geçersiz firmware boyutu veya OTA başlangıç hatası!");
        }
    }
    else
    {
        Serial.printf("[ERROR]: HTTP Hatası: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

void checkForOTAUpdate() // OTA güncelleme kontrol fonksiyonu
{
    Serial.println("\n--- OTA Güncelleme Kontrolü Başlıyor ---");

    if (Firebase.RTDB.getString(&fbdo, "UPDATES/PetVet/version"))
    {
        String latestVersion = fbdo.stringData();
        Serial.println("[Info]: Mevcut Versiyon: " + String(CURRENT_VERSION));
        Serial.println("[Info]: Sunucudaki Versiyon: " + latestVersion);

        if (latestVersion != CURRENT_VERSION)
        {
            Serial.println("[Info]: Yeni versiyon mevcut! Güncelleme başlatılıyor...");

            if (Firebase.RTDB.getString(&fbdo, "UPDATES/PetVet/url"))
            {
                String firmwareURL = fbdo.stringData();
                performOTAUpdate(firmwareURL);
            }
            else
            {
                Serial.println("[ERROR]: Güncelleme URL'si alınamadı: " + fbdo.errorReason());
            }
        }
        else
        {
            Serial.println("[Info]: Cihaz güncel durumda.");
        }
    }
    else
    {
        Serial.println("[ERROR]: Versiyon bilgisi alınamadı: " + fbdo.errorReason());
    }
}

/////////////////////   ZAMAN TAKİP FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void FeedTimeTrackingFunction()
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

void StartMotor(bool ters_yon)
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

void StopMotor()
{
    digitalWrite(m1_en_pin, LOW);
}

/////////////////////   MOTOR KONTROL FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void TurningControl()
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
        StopMotor();
        StartMotor(false);

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
        StopMotor();

        StartMotor(true);
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

/////////////////////   BUZZER FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void FeederSound()
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

void WaterFeederSound()
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

void EEPROMWrite(int address, int value) // EEPROM'a güvenli bir şekilde yazmak için fonksiyon
{
    EEPROM.writeInt(address, value);
    EEPROM.commit();
}

int EEPROMRead(int address) // EEPROM'dan veri okumak için fonksiyon
{
    return EEPROM.readInt(address);
}

void writeTimetoEEPROM() // Zaman değerlerini EEPROM'a yazma işlemi
{
    if (currentDayofWeek != EEPROMRead(DAY_OF_WEEK_ADDRESS))
    {
        EEPROMWrite(DAY_OF_WEEK_ADDRESS, currentDayofWeek);
        vTaskDelay(5);
        EEPROMWrite(DAY_ADDRESS, currentDay);
        vTaskDelay(5);
        EEPROMWrite(MONTH_ADDRESS, currentMonth);
        vTaskDelay(5);
        EEPROMWrite(YEAR_ADDRESS, currentYear);
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Gün Güncellendi: %d\n", currentDayofWeek);
    }
    if (currentHour != EEPROMRead(HOUR_ADDRESS))
    {
        EEPROMWrite(HOUR_ADDRESS, currentHour);
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Saat Güncellendi: %d\n", currentHour);
        Local_Time_Report = true;
    }
    if (currentMinute != EEPROMRead(MINUTE_ADDRESS))
    {
        EEPROMWrite(MINUTE_ADDRESS, currentMinute);
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Dakika Güncellendi: %d\n", currentMinute);
        if (currentMillis > 60000)
        {
            Serial.println(currentMillis);
        }
    }
    if (currentSecond != EEPROMRead(SECOND_ADDRESS))
    {
        EEPROMWrite(SECOND_ADDRESS, currentSecond);
        vTaskDelay(5);
        Serial.printf("[Info]: Hafızadaki Saniye Güncellendi: %d\n", currentSecond);
    }
    Serial.println();
}

/////////////////////  HARİCİ EEPROM FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////
/*
void writeEEPROM_RTC(int address, int data)
{
    Wire.beginTransmission(EEPROM_ADDRESS); // 24C32'nin adresi
    Wire.write((byte)(address >> 8));       // Yüksek adres byte
    Wire.write((byte)(address & 0xFF));     // Düşük adres byte
    Wire.write((byte)data);                 // Veri
    Wire.endTransmission();
    Serial.println("[Info]: Harici Eeproma Yazıldı.");
}

// EEPROM'dan veri okuma fonksiyonu
int readEEPROM_RTC(int address)
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
    Serial.println("[Info]: Harici Eepromdan Okundu.");

    return data;
}

void Epprom_Update_From_PC()
{
    Serial.println("[Info]: CİHAZIN İLK SAAT KURUKUMU YAPILDI.");

    // Yıl, ay ve günü EEPROM'a yazma
    writeEEPROM_RTC(60, 24); // Yılı iki haneli olarak kaydet (örneğin 2024 -> 24)
    vTaskDelay(5);
    writeEEPROM_RTC(50, 8); // Ayı kaydet
    vTaskDelay(5);
    writeEEPROM_RTC(40, 17); // Günü kaydet
    vTaskDelay(5);
    writeEEPROM_RTC(70, 6);
    vTaskDelay(5);

    // Saat, dakika ve saniyeyi EEPROM'a yazma
    writeEEPROM_RTC(10, 0); // Saati kaydet
    vTaskDelay(5);
    writeEEPROM_RTC(20, 0); // Dakikayı kaydet
    vTaskDelay(5);
    writeEEPROM_RTC(30, 15); // Saniyeyi kaydet
    vTaskDelay(5);
}
*/

/////////////////////   ZAMAN YÖNETİM FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void NTPTimeUpdate()
{
    timeClient.update();

    currentDayofWeek = timeClient.getDay();
    currentYear = 24;
    currentMonth = 12;
    currentDay = 10;
    /*
    currentYear = EEPROMRead(YEAR_ADDRESS);
    currentMonth = EEPROMRead(MONTH_ADDRESS);
    currentDay = EEPROMRead(DAY_ADDRESS);

    currentYear = timeClient.getYear();
    currentMonth = timeClient.getMonth();
    currentDay = timeClient.getDay();
    */
    currentSecond = timeClient.getSeconds();
    currentMinute = timeClient.getMinutes();
    currentHour = timeClient.getHours();
}

void NTPTimeUpdateOffline()
{
    // [BUILDING] = Buraya eepromdan okudugu degeri timeclient sunucusunun varsayılan başlatma ayarı olarak ata.

    NTPTimeUpdate();

    Serial.println("[INFO]: ÖNCESİ:");
    Serial.print("currentDayofWeek: ");
    Serial.println(currentDayofWeek);
    Serial.print("currentHour: ");
    Serial.println(currentHour);
    Serial.print("currentMinute: ");
    Serial.println(currentMinute);
    Serial.print("currentSecond: ");
    Serial.println(currentSecond);

    if (!LocalClockControl)
    {
        currentYearEEPROM = EEPROMRead(YEAR_ADDRESS);
        currentMonthEEPROM = EEPROMRead(MONTH_ADDRESS);
        currentDayEEPROM = EEPROMRead(DAY_ADDRESS);
        currentDayofWeekEEPROM = EEPROMRead(DAY_OF_WEEK_ADDRESS);
        currentSecondEEPROM = EEPROMRead(SECOND_ADDRESS);
        currentMinuteEEPROM = EEPROMRead(MINUTE_ADDRESS);
        currentHourEEPROM = EEPROMRead(HOUR_ADDRESS);

        LocalClockControl = true;
    }
    else
    {
    }

    ////////////////////////////////////////////////////// SECOND

    if (currentSecond < currentSecondEEPROM)
    {
        seconddiffent = currentSecondEEPROM - currentSecond;
        currentSecond = currentSecond + seconddiffent;
    }
    else
    {
        seconddiffent = currentSecond - currentSecondEEPROM;
        currentSecond = currentSecond + seconddiffent;
    }
    if (currentSecond == 60)
    {
        currentSecond = currentSecond - 59;
        currentMinute = currentMinute + 1;
    }

    ////////////////////////////////////////////////////// MINUTE

    if (currentMinute < currentMinuteEEPROM)
    {
        minutediffent = currentMinuteEEPROM - currentMinute;
        currentMinute = currentMinute + minutediffent;
    }
    else
    {
        minutediffent = currentMinute - currentMinuteEEPROM;
        currentMinute = currentMinute + minutediffent;
    }
    if (currentMinute > 59)
    {
        currentMinute = currentMinute - 59;
        currentHour = currentHour + 1;
    }

    ////////////////////////////////////////////////////// HOUR

    if (currentHour < currentHourEEPROM)
    {
        hourdiffent = currentHourEEPROM - currentHour;
        currentHour = currentHour + hourdiffent;
    }
    else
    {
        hourdiffent = currentHour - currentHourEEPROM;
        currentHour = currentHour + hourdiffent;
    }
    if (currentHour > 23)
    {
        currentHour = currentHour - 23;
        currentDayofWeek = currentDayofWeek + 1;
    }

    ////////////////////////////////////////////////////// DOW

    if (currentDayofWeek < currentDayofWeekEEPROM)
    {
        DOWdiffent = currentDayofWeekEEPROM - currentDayofWeek;
        currentDayofWeek = currentDayofWeek + DOWdiffent;
    }
    else
    {
        DOWdiffent = currentDayofWeek - currentDayofWeekEEPROM;
        currentDayofWeek = currentDayofWeek + DOWdiffent;
    }
    if (currentDayofWeek > 7)
    {
        currentDayofWeek = currentDayofWeek - 7;
    }

    currentYearEEPROM = currentYear;
    currentMonthEEPROM = currentMonth;
    currentDayEEPROM = currentDay;
    currentDayofWeekEEPROM = currentDayofWeek;
    currentHourEEPROM = currentHour;
    currentMinuteEEPROM = currentMinute;
    currentSecondEEPROM = currentSecond;

    Serial.println("[INFO]: SONRASI:");
    Serial.print("currentDayofWeek: ");
    Serial.println(currentDayofWeek);
    Serial.print("currentHour: ");
    Serial.println(currentHour);
    Serial.print("currentMinute: ");
    Serial.println(currentMinute);
    Serial.print("currentSecond: ");
    Serial.println(currentSecond);
}

void RTCTimeUpdate()
{
    rtc.refresh();

    currentDayofWeek = rtc.dayOfWeek();
    currentYear = rtc.year();
    currentMonth = rtc.month();
    currentDay = rtc.day();
    currentSecond = rtc.second();
    currentMinute = rtc.minute();
    currentHour = rtc.hour();

    if (rtcModel == URTCLIB_MODEL_DS1307 || rtcModel == URTCLIB_MODEL_DS3232)
    {
        position++;
    }
    Serial.flush();
}

/////////////////////   removeErrorCode FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////   SerialPortIncomingControl FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void SerialPortIncomingControl()
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

/////////////////////   DATABASE'E GÖNDERİLEN FONKSİYONLARI    ////////////////////////////////////////////////////////////////////////////////////////////

void ReadyToSendDatas1()
{
    if (manuel_water_status_report) // manuel su alma komutunu aldıktan sonra geri bildirim olarak komutu tekrar false yapıyor.
    {
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/programs/manuel/manuelwater";
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
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/programs/manuel/manuelfeed";
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

void ReadyToSendDatas2()
{
    if (Local_Time_Report)
    {
        // Veri yolu oluştur
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/lastreport";

        // Tarih ve saati uygun formatta birleştir
        String TarihSaat = String(currentDay) + "/" + String(currentMonth) + "/" + String(currentYear) + " " +
                           String(currentHour) + ":" + String(currentMinute) + ":" + String(currentSecond);

        // Firebase'e gönder ve sonucunu kontrol et
        if (Firebase.RTDB.setString(&fbdo, VeriYolu, TarihSaat))
        {
            Local_Time_Report = false; // Gönderim başarılı ise raporlama bayrağını sıfırla
        }
        else
        {
            // Hata durumunda mesaj yazdır
            Serial.println("[ERROR]: Connection FAILED.");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        vTaskDelay(1); // Sistem dengesini korumak için kısa bir gecikme
    }

    ///////////////////////////////////////////////////////////////////////

    if (alarm_status)
    {
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/alarmcode";
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
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/lastmanuelfeedtime";

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
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/foodlevel";
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
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/waterlevel";
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
        VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/batterylevel";
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