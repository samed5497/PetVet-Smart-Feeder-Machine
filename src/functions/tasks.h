#include <functions/setup.h>

// ****************************************
// TASKS ***********************************

void TaskOTA(void *parameter) // Task: OTA güncelleme kontrolü
{
    const TickType_t dailyDelay = pdMS_TO_TICKS(24 * 60 * 60 * 1000); // 1 gün
    // const TickType_t dailyDelay = pdMS_TO_TICKS(7*24 * 60 * 60 * 1000); // 1 hafta
    // const TickType_t dailyDelay = pdMS_TO_TICKS(4 * 7 * 24 * 60 * 60 * 1000); // 1 ay

    TickType_t lastCheckTime = xTaskGetTickCount(); // Son kontrol zamanı
    extern bool ManuelUpdateControl;                // Harici bir değişken olduğunu varsayıyoruz

    while (true)
    {
        if (millis() > 60000 && !SetupMode)
        {
            if (ManuelUpdateControl) // Manuel güncelleme tetiklenirse
            {
                if (!updating && WiFi.status() == WL_CONNECTED)
                {
                    checkForOTAUpdate();
                }
            }
            else // Manuel tetikleme yoksa, günlük kontrol yap
            {
                TickType_t currentTime = xTaskGetTickCount();
                if ((currentTime - lastCheckTime) >= dailyDelay) // Günlük kontrol
                {
                    if (!updating && WiFi.status() == WL_CONNECTED)
                    {
                        checkForOTAUpdate();
                        lastCheckTime = xTaskGetTickCount(); // Son kontrol zamanını güncelle
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Her döngüde 1 saniye bekle
    }
}

void TaskServerConnection(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode && WiFi.status() == WL_CONNECTED) // Güncelleme sırasında raporlama yapma
        {
            if (Firebase.ready() && signupOK)
            {

                ReadyToSendDatas1(); // Değişmesi gereken data vars ona bakacak.

                // Firebase'den tüm gerekli verileri tek seferde çek
                VeriYolu.clear();
                VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID);

                if (Firebase.RTDB.getJSON(&fbdo, VeriYolu))
                {
                    Serial.printf("[Info]: //////////// Data Received!\n\n");

                    DynamicJsonDocument doc(4096); // Geniş bir bellek ayır, çünkü tek seferde çok veri çekiliyor
                    deserializeJson(doc, fbdo.jsonString());

                    // Device verilerini çek ve ilgili değişkenlere ata
                    if (doc.containsKey("device"))
                    {
                        JsonObject device = doc["device"];

                        SERVER_last_manuel_mod_feed_time = device["lastmanuelfeedtime"].as<String>();
                        SERVER_Food_Level = device["foodlevel"].as<int>();
                        SERVER_Water_Level = device["waterlevel"].as<int>();
                        SERVER_batterylevel = device["batterylevel"].as<int>();
                    }

                    // Programs verilerini çek ve ilgili değişkenlere ata
                    if (doc.containsKey("programs"))
                    {
                        JsonObject programs = doc["programs"];
                        // Oto programlarını çek ve ilgili değişkenlere ata
                        if (programs.containsKey("oto"))
                        {
                            JsonObject otoPrograms = programs["oto"];
                            String programKey;
                            for (int i = 1; i <= PROGRAM_SAYISI; i++)
                            {
                                programKey = "program" + String(i);
                                if (otoPrograms.containsKey(programKey))
                                {
                                    feed_program[i] = otoPrograms[programKey]["number"].as<int>();
                                    feed_time_hour[i] = otoPrograms[programKey]["hour"].as<int>();
                                    feed_time_minute[i] = otoPrograms[programKey]["minute"].as<int>();
                                    target_motor_lap_count[i] = (otoPrograms[programKey]["portion"].as<int>() * manuel_feeder_portion);
                                    status[i] = otoPrograms[programKey]["status"].as<int>();
                                }
                            }

                            // Waterrefleshtime programını çek ve ilgili değişkenlere ata
                            String waterkey = "waterrefleshtime";
                            if (otoPrograms.containsKey(waterkey))
                            {
                                int newWaterWorkTime = otoPrograms[waterkey].as<int>();
                                // Gelen veri ile mevcut veri farklı ise güncelle
                                if (water_work_time != newWaterWorkTime)
                                {
                                    water_work_time = newWaterWorkTime;
                                }
                            }
                        }

                        // Manuel program verilerini çek ve ilgili değişkenlere ata
                        if (programs.containsKey("manuel"))
                        {
                            JsonObject manuelProgram = programs["manuel"];
                            manuel_feeder_status = manuelProgram["manuelfeed"].as<bool>();
                            manuel_feeder_portion = manuelProgram["feederportion"].as<int>();
                            manuel_water_status = manuelProgram["manuelwater"].as<bool>();
                            manuel_water_portion = manuelProgram["waterportion"].as<int>();
                        }
                    }

                    // Settings verilerini çek ve ilgili değişkenlere ata
                    if (doc.containsKey("settings"))
                    {
                        JsonObject settings = doc["settings"];
                        String serialkey = "serialport";
                        if (settings.containsKey(serialkey))
                        {
                            bool newReportMode = settings[serialkey].as<bool>();
                            // Gelen veri ile mevcut veri farklı ise güncelle
                            if (report_mode != newReportMode)
                            {
                                report_mode = newReportMode;
                            }
                        }

                        // Volume verilerini çek ve ilgili değişkene ata
                        String volumekey = "soundvolume";
                        if (settings.containsKey(volumekey))
                        {
                            int newVolume = settings[volumekey].as<int>();
                            if (Speaker_Volume != newVolume) // Eğer gelen veri mevcut veri ile farklı ise
                            {
                                Speaker_Volume = newVolume;
                                audio.setVolume(map(Speaker_Volume, 0, 100, 0, 21)); // 0-100 aralığındaki değeri 0-21 aralığına çevir ve ses seviyesini ayarla
                            }
                        }

                        // manuelupdatecontrol verilerini çek ve ilgili değişkene ata
                        String manuelupdatecontrolkey = "manuelupdatecontrol";
                        if (settings.containsKey(manuelupdatecontrolkey))
                        {
                            ManuelUpdateControl = settings[manuelupdatecontrolkey].as<bool>();
                        }

                        // resetcount verilerini çek ve ilgili değişkene ata
                        String resetcountkey = "resetcount";
                        if (settings.containsKey(resetcountkey))
                        {
                            ResetCount = settings[resetcountkey].as<bool>();
                            if (ResetCount)
                            {
                                VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/settings/resetcount";
                                if (Firebase.RTDB.setBool(&fbdo, VeriYolu, false))
                                {
                                    for (int i = 0; i < EEPROM_SIZE; ++i)
                                    {
                                        EEPROM.write(i, 0);
                                    }
                                    EEPROM.commit();
                                    Serial.println("");
                                    Serial.println("[Info]: Ayarlar Sıfırlandı. Cihaz 3 Saniye içinde yeniden başlatılacak.");
                                    Serial.println("");
                                    colorWipe(strip.Color(255, 255, 255), 0);

                                    delay(2000);
                                    ESP.restart();
                                }
                                else
                                {
                                    Serial.print("[ERROR]: ResetCount güncellenemedi. Nedeni: ");
                                    Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                                    Serial.println("Hata Sebebi: " + fbdo.errorReason());
                                }
                            }
                        }
                    }
                }
                else
                {
                    Serial.print("[ERROR]: Connection FAILED. ");
                    Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                    Serial.println("Hata Sebebi: " + fbdo.errorReason());
                }

                ReadyToSendDatas2(); // Gönderilecek datalar

                Serial.printf("[Info]: //////////// Data Sended!\n\n");

                /*
                          if (report_mode)
                          {
                              Serial.println();
                              // Serial.print("Serial Port Durumu        : ");
                              // Serial.println(report_mode);
                              Serial.print("Manuel Water Status      : ");
                              Serial.println(manuel_water_status);
                              Serial.print("Water Loop Time          : ");
                              Serial.println(water_work_time);
                              Serial.println();

                              Serial.print("Manuel Feed Status       : ");
                              Serial.println(manuel_feeder_status);
                              Serial.print("Last Manuel Feed Time    : ");
                              Serial.print(last_manuel_mod_feed_time);
                              Serial.print(" - Portion: ");
                              Serial.println(manuel_feeder_portion);
                              Serial.println();

                                                         // Çekilen verileri seri portta göster
                                                         for (int i = 1; i <= PROGRAM_SAYISI; i++)
                                                         {
                                                             Serial.print(" ~ Program ");
                                                             Serial.print(feed_program[i]);
                                                             Serial.print(" / ");
                                                             Serial.print(feed_time_hour[i]);
                                                             Serial.print(":");
                                                             Serial.print(feed_time_minute[i]);
                                                             Serial.print(" - Portion: ");
                                                             Serial.print(target_motor_lap_count[i]);
                                                             Serial.print(" / Status: ");
                                                             Serial.println(status[i]);
                                                         }

                          }
                          */
            }
        }
        vTaskDelay(60000 / portTICK_PERIOD_MS); // 1 Hz
    }
}

void TaskTimeControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            currentMillis = millis();

            if (WiFi.status() != WL_CONNECTED) // bağlantı kopması durumundaysa alarmı aç
            {
                wifi_alarm = true;
#if defined(WiFi_clock)
                NTPTimeUpdateOffline();

#elif defined(WiFi_and_RTC_clock)

                RTCTimeUpdate();
#endif
            }

            else if (WiFi.status() == WL_CONNECTED) // WİFİ BAĞLI OLARAK DEVAM EDİYORSA
            {
                NTPTimeUpdate();
                LocalClockControl = false;

                if (wifi_alarm)
                {
                    wifi_alarm = false; // wifi bağlıysa alarmı kapat
                    removeErrorCode(" 100 -");
                }
            }

            if (currentMillis > 7500 && currentMillis < 30000 and first_time_update)
            {
                EEPROM_Time_Update = true;
                first_time_update = false;
            }
            if (currentMillis - previousMinuteMillis >= 60000) // MINUTE_INTERVAL
            {
                previousMinuteMillis = currentMillis;
                EEPROM_Time_Update = true;
            }

            if (EEPROM_Time_Update) // //  hafızadaki veriyi güncelle
            {
                writeTimetoEEPROM();
                EEPROM_Time_Update = false;
            }

            FeedTimeTrackingFunction(); // Kontrol edilecek zamanlara göre besleme modunu ayarla
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 saniyede bir güncelleme
    }
}

void TaskMicControl(void *pvParameters)
{
    // [BUILDING] = bu kısım tamamen kontrol edilip havlama tespiti yapılacak.
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            noisy = analogRead(mic_pin);

            if (noisy > esikdeger)
            {
                if (noisy_check == 0)
                {
                    noisy_check = 1;
                }
                else if (noisy_check == 1)
                {
                    noisy_check = 2;
                }
                else if (noisy_check == 2)
                {
                    noisy_check = 3;
                }
                else if (noisy_check == 3)
                {
                    noisy_alarm = true;
                    noisy_check = 0;
                }
            }
            else
            {
                noisy_alarm = false;

                removeErrorCode(" 150 -");
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS); // 2 Hz
    }
}

void TaskSerialPortReport(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (report_mode)
            {
                if (report)
                {
                    Serial.begin(115200);
                    report = false;
                    Serial.println("[Info]: Serial Port Active!");
                    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
                    vTaskDelay(100);        // 100 milisaniye bekle
                    noTone(buzzer_pin);     // Sesi kapat
                    Serial.println();
                }

                Serial.printf("----------  Saat: %02d:%02d:%02d *** Tarih: %02d/%02d/20%02d *** %d.gün\n\n",
                              currentHour, currentMinute, currentSecond, currentDay, currentMonth, currentYear, currentDayofWeek);

                if (hata_kodu.length() > 2)
                {
                    Serial.printf("***[ERROR]: %s\n\n", hata_kodu.c_str());
                }
                Serial.printf(" ~ Food            : %% %d\n", Food_Level);
                Serial.printf(" ~ Water           : %% %d (%d)\n", Water_Level, wtr_lvl);
                Serial.printf(" ~ Volume          : %% %d\n", Speaker_Volume);
                Serial.printf(" ~ Noisy           : %d\n", noisy);
                Serial.printf(" ~ Güç             : %% %d   %.1fv / %s\n\n",
                              (int)batterylevel, V_Battery, charge_mod ? "Şarj Ediliyor." : "Pil Gücünde.");

                ////////////////////////////////////// SYSTEM
                int32_t rssi = WiFi.RSSI();
                Serial.printf(" ~ WiFi            : %S / RSSI: %d dBm\n", ssid_STA, rssi);

                // RAM Bilgisi
                uint32_t freeHeap = ESP.getFreeHeap(); // İşlemcideki'daki mevcut RAM bilgisi
                Serial.printf(" ~ Free RAM        : %d KB\n", freeHeap / 1024);

                // ROM Bilgisi (Flash Bellek)
                uint32_t totalFlash = ESP.getFlashChipSize();
                uint32_t sketchSize = ESP.getSketchSize();
                uint32_t freeFlash = ESP.getFreeSketchSpace();
                float flashUsage = (float)(sketchSize) / totalFlash * 100.0;

                Serial.printf(" ~ Total Flash Size: %d KB\n", totalFlash / 1024);
                Serial.printf(" ~ Used Flash      : %d KB\n", sketchSize / 1024);
                Serial.printf(" ~ Free Flash      : %d KB (%% %.1f)\n\n", freeFlash / 1024, 100.0 - flashUsage);
                Serial.printf(" ~ Device ID       : %d\n\n", DEVICE_ID);
            }
            else
            {
                if (!report)
                {
                    Serial.println("[Info]: Serial Port Disabled.");
                    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
                    vTaskDelay(100);        // 100 milisaniye bekle
                    noTone(buzzer_pin);     // Sesi kapat
                    Serial.flush();
                    Serial.end();
                    report = true;
                }
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 saniye bekle
    }
}

void TaskFeeder(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (feed_mode)
            {
                FeederSound();

                ilk_temas = millis();
                son_tiklama_zamani = millis();
                StartMotor(true);

                while (motor_lap_count < target_motor_lap_count[runing_program]) // Her programın porsiyonu, manuel porsyonun tur sayısıyla çarpılıyor.
                {
                    TurningControl();
                }

                son_temas = millis();
                Serial.print("FARK. :");
                Serial.println(son_temas - ilk_temas);

                while (digitalRead(switch_pin) == HIGH)
                {
                    StartMotor(true);
                }

                vTaskDelay(10 / portTICK_PERIOD_MS); // 100 Hz

                while (digitalRead(switch_pin) == HIGH)
                {
                    StartMotor(true);
                }
                Serial.print("DURDU. :");
                Serial.println(millis());
                Serial.println();

                StopMotor();
                motor_lap_count = 0;

                feed_mode = false;
            }

            if (manuel_feeder_status)
            {
                Serial.println("[INFO]: Feeding Time. Program: 0");
                Serial.println();
                feed_mode = true;
                manuel_feeder_status_report = true;
                manuel_feeder_status = false;

                // Değişkenleri String'e dönüştür
                String yearStr = String(currentYear);
                String monthStr = String(currentMonth);
                String dayStr = String(currentDay);
                String hourStr = String(currentHour);
                String minuteStr = String(currentMinute);
                String secondStr = String(currentSecond);

                // Tarih ve saati uygun formatta birleştir
                String Saat = yearStr + "/" + monthStr + "/" + dayStr + " " +
                              hourStr + ":" + minuteStr + ":" + secondStr;

                last_manuel_mod_feed_time = Saat;

                // FeederSound();

                target_motor_lap_count[0] = manuel_feeder_portion; // manuel butona basılınca tek porsiyon kadar besleme yapılıyor.
                runing_program = 0;
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // 100 Hz
    }
}

void TaskFeederFlowControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (feed_mode == true)
            {
                control = true;
                sayac = millis();
            }

            while (control && (millis() - sayac < 30000))
            {
                // [BUILDING] Burada motor çalıştıktan sonra 30 saniye kadar akış olup olmadığını kontrol ediyoruz.
                // Bu döngü 30 saniye boyunca çalışacak ve vTaskDelay ile belirli aralıklarla gecikme yaparak CPU'yu meşgul etmeyecek.
                vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
            }

            // 30 saniye dolduğunda kontrol değişkenini false yapar ve döngüden çıkar
            control = false;

            // Diğer işlemler burada yapılabilir
            // ...
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
    }
}

void TaskSoundControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode && WiFi.status() == WL_CONNECTED) // Güncelleme sırasında raporlama yapma
        {

            if (Speaker_Volume > 0)
            {
                if (!SoundFirstOpen)
                {
                    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
                    SoundFirstOpen = true;
                    Serial.println("[Info]: Music server connected!");
                }
                // [BUILDING] Buraya uzaktan ses gönderdiğimizde çalması için sunucu bağlantıları yapılacak.

                audio.loop(); // Ses döngüsü işlemi, eğer ses seviyesi 0'dan büyükse çalıştır
            }
            else
            {
                SoundFirstOpen = false;

                audio.stopSong(); // Ses seviyesini 0 olduğu için sesi durdur
            }
        }

        vTaskDelay(1 / portTICK_PERIOD_MS); // 1 ms gecikme (1000 Hz)
    }
}

void TaskBatteryControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            float totalVoltage = 0.0;
            int ornekleme_sayisi = 100;

            for (int i = 0; i < ornekleme_sayisi; i++)
            {
                totalVoltage += (analogRead(batt_pin + 0.1)); // normalde 0.55 v diyot düşümü var ancak orantılayınca bu sekilde oluyor.
                vTaskDelay(5);                                // Her okuma arasında kısa bir gecikme
            }

            float avgReading = totalVoltage / ornekleme_sayisi;
            double r_ust = 470000.0;
            double r_alt = 81500.0; // 100000

            // ADC değerini voltaja çeviréé
            V_Battery = (avgReading / 1023.0) * ((r_ust + r_alt) / r_alt);

            if (V_Battery > 4.2)
            {
                charge_mod = true;
            }
            else
            {
                charge_mod = false;
            }
            // Batarya doluluk oranını hesapla
            batterylevel = (((V_Battery - 3.3) / (4.15 - 3.3)) * 100);

            if (batterylevel > 100)
            {
                batterylevel = 100;
            }
            batterylevel = constrain(batterylevel, 0, 100); // Doluluk oranını 0% ile 100% arasında sınırla
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS); // 1 Hz
    }
}

void TaskAlarmControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (!sikisma_alarm && !max_water_alarm)
            {
                noTone(buzzer_pin); // Sesi kapat

                if (min_water_alarm)
                {
                    // Eğer hata_kodu içinde "125" kodu yoksa ekle
                    if (hata_kodu.indexOf("125") == -1)
                    {
                        hata_kodu += " 125 -";
                        Serial.println("[ALARM]: Min Water Alarm!");
                        Serial.println();
                        // tone(buzzer_pin, 2000); // 2000 Hz frekansta ses üret
                    }
                }

                if (wifi_alarm)
                {
                    // Eğer hata_kodu içinde "100" kodu yoksa ekle
                    if (hata_kodu.indexOf("100") == -1)
                    {
                        hata_kodu += " 100 -";
                        Serial.println("[ALARM]: Wi-Fi Connection Error!");
                        Serial.println();
                    }
                }
                if (noisy_alarm)
                {
                    // Eğer hata_kodu içinde "150" kodu yoksa ekle
                    if (hata_kodu.indexOf("150") == -1)
                    {
                        hata_kodu += " 150 -";
                        Serial.print("[ALARM]: Gürültü Alarm! - ");
                        Serial.println(noisy);
                        Serial.println();
                    }
                }
            }
            else
            {
                if (sikisma_alarm)
                {
                    // Eğer hata_kodu içinde "110" kodu yoksa ekle
                    if (hata_kodu.indexOf("110") == -1)
                    {
                        hata_kodu += " 110 -";
                        Serial.println("[ALARM]: Sıkışma Alarm!");
                        Serial.println();
                        tone(buzzer_pin, 500); // 500 Hz frekansta ses üret
                    }
                }

                if (max_water_alarm)
                {
                    // Eğer hata_kodu içinde "120" kodu yoksa ekle
                    if (hata_kodu.indexOf("120") == -1)
                    {
                        hata_kodu += " 120 -";
                        Serial.println("[ALARM]: Max Water Alarm!");
                        Serial.println();
                        // tone(buzzer_pin, 2000); // 2000 Hz frekansta ses üret
                    }
                }
            }

            if (hata_kodu.length() > 2)
            {
                alarm_status = true;
            }
            else
            {
                alarm_status = false;
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 Hz
    }
}

void TaskButtonControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (digitalRead(button_pin) == LOW) // Butona basıldığında
            {

                if (buttonPressStartTime == 0) // Basılma zamanı kaydedilmemişse kaydet
                {
                    buttonPressStartTime = millis();
                }
                if (!buttonHeld && millis() - buttonPressStartTime > longPressDuration) // 1 saniye boyunca basılı tutulduysa
                {
                    buttonHeld = true;
                    manuel_water_status = true; // Butona uzun basıldığında manuel_water_status true olacak
                    // WaterFeederSound();
                }
            }
            else // Buton bırakıldığında
            {
                if (buttonHeld)
                {
                    manuel_water_status = false; // Buton bırakıldığında manuel_water_status false olacak
                }
                else if (buttonPressStartTime != 0 && millis() - buttonPressStartTime < longPressDuration)
                {
                    // Buton kısa süreliğine basıldıysa
                    Serial.println("[INFO]: Feeding Time. Program: 0");
                    Serial.println();

                    feed_mode = true;

                    // Değişkenleri String'e dönüştür
                    String yearStr = String(currentYear);
                    String monthStr = String(currentMonth);
                    String dayStr = String(currentDay);
                    String hourStr = String(currentHour);
                    String minuteStr = String(currentMinute);
                    String secondStr = String(currentSecond);

                    // Tarih ve saati uygun formatta birleştir
                    String Saat = yearStr + "/" + monthStr + "/" + dayStr + " " +
                                  hourStr + ":" + minuteStr + ":" + secondStr;

                    last_manuel_mod_feed_time = Saat;

                    // FeederSound();

                    target_motor_lap_count[0] = manuel_feeder_portion; // manuel butona basılınca tek porsiyon kadar besleme yapılıyor.
                    runing_program = 0;
                }

                buttonPressStartTime = 0; // Zamanlayıcıyı sıfırla
                buttonHeld = false;
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
    }
}

void TaskLightControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (feed_mode)
            {
                // colorWipe(strip.Color(LED_R, LED_G, LED_B), 0);
                colorWipe(strip.Color(0, 255, 0), 0);
            }
            else if (sikisma_alarm)
            {
                // colorWipe(strip.Color(LED_R, LED_G, LED_B), 0);
                colorWipe(strip.Color(255, 255, 0), 0);
            }
            else if (wifi_alarm)
            {
                bool led = !led;
                if (led)
                {
                    colorWipe(strip.Color(255, 0, 0), 0);
                }
                else
                {
                    colorWipe(strip.Color(0, 0, 0), 0);
                }
            }
            else if (min_water_alarm)
            {
                bool led2 = !led2;
                if (led2)
                {
                    colorWipe(strip.Color(255, 0, 255), 0);
                }
                else
                {
                    colorWipe(strip.Color(0, 0, 0), 0);
                }
            }

            else
            {
                // colorWipe(strip.Color(LED_R, LED_G, LED_B), 0);

                colorWipe(strip.Color(0, 0, 200), 0);
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
    }
}

void TaskWaterFeeder(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            if (millis() - last_water_time > (water_work_time * 60000))
            {
                Serial.println("[Info]: Su sayacı sıfırandı. Su verilliyor...");
                Serial.println();

                // WaterFeederSound();

                unsigned long manuel_water_time = millis();

                while (millis() - manuel_water_time < (manuel_water_portion * 1000))
                {
                    digitalWrite(m2_en_pin, HIGH);
                    vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
                }
                manuel_water_status = false;

                Serial.println("[Info]: Su verildi!");
                Serial.println();

                digitalWrite(m2_en_pin, LOW);
                last_water_time = millis();
            }

            if (manuel_water_status)
            {
                Serial.println("[Info]: Manuel Su veriliyor.");
                // WaterFeederSound();
                unsigned long manuel_water_time = millis();

                while (millis() - manuel_water_time < (manuel_water_portion * 1000))
                {
                    digitalWrite(m2_en_pin, HIGH);
                }
                manuel_water_status_report = true; // EK - Su besleme bittilkten sonra sunucudaki ayarı da değiştirme kodu eklenecek:
                manuel_water_status = false;
            }
            else
            {
                digitalWrite(m2_en_pin, LOW);
            }
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // 10 Hz
    }
}

void TaskWaterLevelControl(void *pvParameters)
{
    int max_water_value = 2000;
    int min_water_value = 600;
    int sample_count = 200; // Örnekleme sayısı

    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            int sum_of_samples = 0;

            // 100 adet örnekleme al ve toplamlarını hesapla
            for (int i = 0; i < sample_count; i++)
            {
                sum_of_samples += analogRead(weter_level_pin);
                vTaskDelay(10 / portTICK_PERIOD_MS); // 5 ms bekle
            }

            // Ortalama değeri hesapla
            wtr_lvl = sum_of_samples / sample_count;
            Water_Level = map(wtr_lvl, min_water_value, max_water_value, 0, 100);
            Water_Level = constrain(Water_Level, 0, 100);
            /*
                        // Alarm durumlarını kontrol et
                        if (wtr_lvl > (max_water_value + 200))
                        {
                            max_water_alarm = true;
                        }
                        else if (wtr_lvl < 700 && wtr_lvl > 300)
                        {
                            min_water_alarm = true;
                        }
                        else
                        {
                            min_water_alarm = false;
                            max_water_alarm = false;
                            removeErrorCode(" 125 -");
                            removeErrorCode(" 120 -");
                        }
            */
        }

        vTaskDelay(2500 / portTICK_PERIOD_MS); // 1 Hz döngü
    }
}

void TaskFoodLevelControl(void *pvParameters)
{
    while (true)
    {
        if (!updating && !SetupMode) // Güncelleme sırasında raporlama yapma
        {
            //[BUILDING] = bu kısım için sensor eklenip denencek.
        }
        vTaskDelay(60000 / portTICK_PERIOD_MS); //  Hz
    }
}

/*
void Task(void *pvParameters)
{
    while (true)
    {
    if (!updating) // Güncelleme sırasında raporlama yapma
        {
            //[BUILDING] =
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // 100 Hz
    }
}
*/