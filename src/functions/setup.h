#include <functions/aux_funcs.h>

void FirebaseDeviceIDAssignment()
{
    // Başlangıç değişkenleri
    String currentMAC = WiFi.macAddress(); // Mevcut cihazın MAC adresi
    bool isMatched = false;                // MAC adresi eşleşme durumu

    // Firebase hazır ve kimlik doğrulama tamamlandıysa
    if (Firebase.ready() && signupOK)
    {
        // Mevcut cihazları kontrol etmek için döngü
        while (true)
        {
            String devicePath = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/macadress";
            // Firebase'den cihazın MAC adresini al
            if (Firebase.RTDB.getString(&fbdo, devicePath))
            {
                // MAC adresi eşleşiyor mu?
                if (fbdo.stringData() == currentMAC)
                {
                    Serial.print("[Info]: Mevcut cihaz bulundu. ");
                    isMatched = true;
                    break; // Döngüden çık
                }
            }
            else if (fbdo.httpCode() == -103)
            {
                // Yeni cihaz için bir yol bulunamadıysa, yeni cihaz ataması yapılır
                Serial.println("[Info]: Yeni cihaz oluşturuluyor.");
                break; // Döngüden çık
            }
            else
            {
                // Diğer hata durumları
                Serial.print("[ERROR]: Firebase bağlantı hatası. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());

                return;
            }

            DEVICE_ID++; // Sonraki cihaz numarasına geç
        }

        // Eğer eşleşme yoksa yeni cihaz kaydı oluştur
        if (!isMatched)
        {
            FirebaseJson json;

            // Device bilgilerini ekle
            json.set("/device/macadress", currentMAC);
            json.set("/device/lastmanuelfeedtime", last_manuel_mod_feed_time);
            json.set("/device/foodlevel", Food_Level);
            json.set("/device/waterlevel", Water_Level);
            json.set("/device/batterylevel", batterylevel);
            json.set("/device/alarmcode", "-");
            json.set("/device/lastmanuelfeedtime", "-");
            json.set("/device/ipadress", WiFi.localIP().toString().c_str());
            json.set("/device/currentversion", CURRENT_VERSION);

            // Settings bilgilerini ekle
            json.set("/settings/serialport", report_mode);
            json.set("/settings/soundvolume", Speaker_Volume);
            json.set("/settings/manuelupdatecontrol", ManuelUpdateControl);
            json.set("/settings/resetcount", ResetCount);

            // Manuel ayarlarını ekle
            json.set("/programs/manuel/feederportion", manuel_feeder_portion);
            json.set("/programs/manuel/waterportion", manuel_water_portion);
            json.set("/programs/manuel/manuelfeed", manuel_feeder_status);
            json.set("/programs/manuel/manuelwater", manuel_water_status);
            json.set("/programs/oto/waterrefleshtime", water_work_time);

            // Program ayarlarını ekle
            for (int i = 1; i <= PROGRAM_SAYISI; i++)
            {
                String programPath = "programs/oto/program" + String(i);
                json.set(programPath + "/number", i);
                json.set(programPath + "/hour", 0);
                json.set(programPath + "/minute", 0);
                json.set(programPath + "/portion", 2);
                json.set(programPath + "/status", false);
            }

            // Firebase'e gönder
            String devicePath = uid + "/PetVet/DEVICE" + String(DEVICE_ID);
            if (Firebase.RTDB.setJSON(&fbdo, devicePath, &json))
            {
                Serial.println("[Info]: Yeni cihaz kaydı başarıyla oluşturuldu.");
            }
            else
            {
                Serial.print("[ERROR]: Firebase JSON gönderimi başarısız. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
                return;
            }
        }

        // Cihaz ID'sini belirle
        DEVICE_ID = DEVICE_ID;
        Serial.printf("Cihaz ID: %d\n", DEVICE_ID);
    }
    else
    {
        Serial.println("[ERROR]: Firebase bağlantısı hazır değil.");
    }

    //////////// Kullanıcı Mail kontrolü ve yazma ////////////
    VeriYolu = uid + "/zzz_mail";
    if (Firebase.RTDB.getString(&fbdo, VeriYolu))
    {
        String mevcutDeger = fbdo.stringData();
        if (mevcutDeger == email)
        {
            // Serial.println("[Info]: Mail zaten aynı, değişiklik yapılmadı.");
        }
        else
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, email))
            {
                Serial.println("[Info]: Mail güncellendi.");
            }
            else
            {
                Serial.print("[ERROR]: Mail güncellenemedi. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
    }
    else
    {
        if (fbdo.httpCode() == -103)
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, email))
            {
                Serial.println("[Info]: Mail oluşturuldu.");
            }
            else
            {
                Serial.print("[ERROR]: Mail oluşturulamadı. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
        else
        {
            Serial.print("[ERROR]: Mail kontrolü başarısız. ");
            Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
            Serial.println("Hata Sebebi: " + fbdo.errorReason());
        }
    }

    //////////// Kullanıcı Şifre kontrolü ve yazma ////////////
    VeriYolu = uid + "/zzz_mpass";
    if (Firebase.RTDB.getString(&fbdo, VeriYolu))
    {
        String mevcutDeger = fbdo.stringData();
        if (mevcutDeger == email_pass)
        {
            // Serial.println("[Info]: Mail şifresi zaten aynı, değişiklik yapılmadı.");
        }
        else
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, email_pass))
            {
                Serial.println("[Info]: Mail şifresi güncellendi.");
            }
            else
            {
                Serial.print("[ERROR]: Mail şifresi güncellenemedi. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
    }
    else
    {
        if (fbdo.httpCode() == -103)
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, email_pass))
            {
                Serial.println("[Info]: Mail şifresi oluşturuldu.");
            }
            else
            {
                Serial.print("[ERROR]: Mail şifresi oluşturulamadı. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
        else
        {
            Serial.print("[ERROR]: Mail şifre kontrolü başarısız. ");
            Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
            Serial.println("Hata Sebebi: " + fbdo.errorReason());
        }
    }

    //////////// Cihaz Versiyon kontrolü ve yazma ////////////
    VeriYolu = uid + "/PetVet/DEVICE" + String(DEVICE_ID) + "/device/currentversion";
    if (Firebase.RTDB.getString(&fbdo, VeriYolu))
    {
        String mevcutDeger = fbdo.stringData();
        if (mevcutDeger == String(CURRENT_VERSION))
        {
            // Serial.println("[Info]: Mail şifresi zaten aynı, değişiklik yapılmadı.");
        }
        else
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, CURRENT_VERSION))
            {
                Serial.println("[Info]: Cihaz Versiyonu güncellendi.");
            }
            else
            {
                Serial.print("[ERROR]:  Cihaz Versiyonu güncellenemedi. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
    }
    else
    {
        if (fbdo.httpCode() == -103)
        {
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, email_pass))
            {
                Serial.println("[Info]: Mail şifresi oluşturuldu.");
            }
            else
            {
                Serial.print("[ERROR]: Mail şifresi oluşturulamadı. ");
                Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
                Serial.println("Hata Sebebi: " + fbdo.errorReason());
            }
        }
        else
        {
            Serial.print("[ERROR]: Mail şifre kontrolü başarısız. ");
            Serial.println("HTTP Kod: " + String(fbdo.httpCode()));
            Serial.println("Hata Sebebi: " + fbdo.errorReason());
        }
    }
}

void AmplificatorStarting()
{
    audio.setPinout(I2S_BCLK_pin, I2S_LRCLK_pin, I2S_DIN_pin);
    audio.setVolume(map(Speaker_Volume, 0, 100, 0, 21)); // 0...21
    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
}

void TimeStartingOffline()
{
    wifi_alarm = true;
#if defined(WiFi_and_RTC_clock)

    Wire.begin();
    // Cihazın bağlı olup olmadığını kontrol et
    Wire.beginTransmission(RTC_ADDRESS);

    if (Wire.endTransmission() == 0)
    { // 0: Cihaz bağlı
        Serial.println("RTC cihazı algılandı.");

        // RTC başlatma işlemleri
        URTCLIB_WIRE.begin();
        rtc.set_rtc_address(RTC_ADDRESS);
        rtc.set_model(rtcModel);
        rtc.refresh();

        Serial.println("RTC başlatıldı.");

        currentDayofWeek = EEPROMRead(DAY_OF_WEEK_ADDRESS);
        currentYear = EEPROMRead(YEAR_ADDRESS);
        currentMonth = EEPROMRead(MONTH_ADDRESS);
        currentDay = EEPROMRead(DAY_ADDRESS);
        currentSecond = EEPROMRead(SECOND_ADDRESS);
        currentMinute = EEPROMRead(MINUTE_ADDRESS);
        currentHour = EEPROMRead(HOUR_ADDRESS);

        Serial.println("[Info]: Cihaz saati Offline olarak ayarlandı.");

        //
        //
        // Yalnızca bir kez kullanın, ardından devre dışı bırakın
        rtc.set(currentSecond, currentMinute, currentHour, currentDayofWeek, currentDay, currentMonth, currentYear);
        // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
        //
        //

        rtc.set_12hour_mode(false);

        if (rtc.enableBattery())
        {
            Serial.println("[Info]: RTC Pil doğru şekilde etkinleştirildi.");
        }
        else
        {
            Serial.println("[Info]: RTC Pil etkinleştirilirken HATA.");
        }

        // OSC'nin VBAT kullanacak şekilde ayarlanıp ayarlanmadığını kontrol edin
        if (rtc.getEOSCFlag())
            Serial.println(F("[Info]: Osilatör, VCC kesildiğinde VBAT'ı kullanmayacaktır. VCC olmadan zaman artmaz!"));
        else
            Serial.println(F("[Info]: Osilatör, VCC kesildiğinde VBAT'ı kullanacaktır."));

        Serial.print("[Info]: Kayıp güç durumu:");

        if (rtc.lostPower())
        {
            Serial.print("GÜÇ KESİLDİ. Bayrak temizleniyor...");
            rtc.lostPowerClear();
            Serial.println(" Tamamlandı.");
        }
        else
        {
            Serial.println(" GÜÇ TAMAM");
        }

        Serial.println();

        // SRAM'ı ayarlayın (DS3231'de SRAM yoktur, dolayısıyla hiçbir şey saklamaz ve her zaman 0xff değerini döndürür)
        for (position = 0; position < 255; position++)
        {
            rtc.ramWrite(position, position);
        }
        position = 0;
    }
    else
    {
        Serial.println("RTC cihazı algılanamadı!");
#define WiFi_clock
#undef WiFi_and_RTC_clock
    }
#elif defined(WiFi_clock)
    NTPTimeUpdateOffline(); //[BUILDING]
#endif
}

void TimeStartingOnline()
{
#if defined(WiFi_and_RTC_clock)
    Wire.begin();
    // Cihazın bağlı olup olmadığını kontrol et
    Wire.beginTransmission(RTC_ADDRESS);
    if (Wire.endTransmission() == 0)
    { // 0: Cihaz bağlı
        Serial.println("RTC cihazı algılandı.");

        // RTC başlatma işlemleri
        URTCLIB_WIRE.begin();
        rtc.set_rtc_address(RTC_ADDRESS);
        rtc.set_model(rtcModel);
        rtc.refresh();

        Serial.println("RTC başlatıldı.");

        NTPTimeUpdate();

        //
        //
        // Yalnızca bir kez kullanın, ardından devre dışı bırakın
        rtc.set(currentSecond, currentMinute, currentHour, currentDayofWeek, currentDay, currentMonth, currentYear);
        // rtc.set(second, minute, hour, dayOfWeek, dayOfMonth, month, year)
        //
        //

        rtc.set_12hour_mode(false);

        if (rtc.enableBattery())
        {
            Serial.println("[Info]: RTC Pil doğru şekilde etkinleştirildi.");
        }
        else
        {
            Serial.println("[Info]: RTC Pil etkinleştirilirken HATA.");
        }

        // OSC'nin VBAT kullanacak şekilde ayarlanıp ayarlanmadığını kontrol edin
        if (rtc.getEOSCFlag())
            Serial.println(F("[Info]: Osilatör, VCC kesildiğinde VBAT'ı kullanmayacaktır. VCC olmadan zaman artmaz!"));
        else
            Serial.println(F("[Info]: Osilatör, VCC kesildiğinde VBAT'ı kullanacaktır."));

        Serial.print("[Info]: Kayıp güç durumu:");
        if (rtc.lostPower())
        {
            Serial.print("GÜÇ KESİLDİ. Bayrak temizleniyor...");
            rtc.lostPowerClear();
            Serial.println(" Tamamlandı.");
        }
        else
        {
            Serial.println(" GÜÇ TAMAM");
        }
        Serial.println();

        // SRAM'ı ayarlayın (DS3231'de SRAM yoktur, dolayısıyla hiçbir şey saklamaz ve her zaman 0xff değerini döndürür)
        for (position = 0; position < 255; position++)
        {
            rtc.ramWrite(position, position);
        }
        position = 0;
    }
    else
    {
#define WiFi_clock
#undef WiFi_and_RTC_clock

        URTCLIB_WIRE.end();
        Wire.end();

        NTPTimeUpdate();
    }

#elif defined(WiFi_clock)
    NTPTimeUpdate();
    Serial.println("[Info]: Cihaz saati Online olarak ayarlandı.");
#endif
}

void StartingSerialandApps()
{
    Serial.begin(115200);
    Serial.println("HMI Başlatılıyor. ");
    Serial.println();
    Serial.println();
    Serial.print(" >>> ");
    Serial.print(UYGULAMA);
    Serial.print(" / ");
    Serial.println("Firmware Version: " CURRENT_VERSION);
    Serial.printf("\r\n");
    vTaskDelay(10);
    Serial.flush();
    digitalWrite(buzzer_pin, HIGH);
    vTaskDelay(250);
    digitalWrite(buzzer_pin, LOW);
    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
    vTaskDelay(250);        // 250 milisaniye bekle
    noTone(buzzer_pin);     // Sesi kapat

    digitalWrite(m1_en_pin, LOW);
    digitalWrite(m2_en_pin, LOW);
    digitalWrite(m2_in1_pin, LOW);
    digitalWrite(m2_in2_pin, HIGH);
    digitalWrite(I2C_gnd, HIGH);

    if (digitalRead(button_pin) == LOW) // EĞER Açılışta besleme butonuna basılı tutuluyorsa resetleme yap.
    {
        delay(1000);
        if (digitalRead(button_pin) == LOW)
        {
            EEPROM.begin(EEPROM_SIZE); // EEPROM'i başlat
            delay(100);
            digitalWrite(buzzer_pin, HIGH);

            for (int i = 0; i < EEPROM_SIZE; ++i)
            {
                EEPROM.write(i, 0);
                EEPROM.commit();
            }
            Serial.println("");
            Serial.println("[Bilgi]: Ayarlar Sıfırlandı. Yeniden yapılandırmak için cihazı yeniden başlatın. ");
            Serial.println("");
            delay(1000);
            ESP.restart();
        }
    }
}

void StartMemoryAndWifiControl()
{
    EEPROM.begin(EEPROM_SIZE); // EEPROM'i başlat

    if (Hafizadan_Yukle()) // Hafzada kayıtlı bilgi varsa buraya gir.
    {
        if (WifiControlAndStart()) // Hafızadaki kayıtlı bilgilerle bağlantı kurmaya çalış.
        {
            if (FirebaseUserControl()) // Hafızadaki kayıtlı bilgilerle bağlantı kurmaya çalış.
            {
                SetupMode = false;
                // StartWebServer();         // Bağlantı kurulursa istenieln web sunucuyu başlat.
                WiFi.softAPdisconnect(true); // AP Modunu devre dışı bırak.
                return;
            }
        }
    }

    SetupMode = true; // Katıtlı veri yoksa kurulum modunu aktifleştir ve AP modunda devam et.
    SetupModeFunction();
}

// ****************************************
// SETUP ***********************************

void setup()
{
    pins();                  // Pin Tanımlamaları
    StartingSerialandApps(); // İlk çalıştırılan uygulama ve seri port mesajları
    StartMemoryAndWifiControl();

    /////////////////////////////////////////////// Başlangıç Uygulamarı Sonrası İlk Fonksiyonlar
    if (!SetupMode)
    {
        AmplificatorStarting(); // MAX98 Entegresini devreye al

        if (WiFi.status() != WL_CONNECTED) // WİFİ BAĞLANTISI OLMADIYSA BU FONKSYONLARI YAP.
        {
            Serial.println();
            Serial.println("[ALARM]: WiFi connection failed, using EEPROM time");
            Serial.println();

            wifi_alarm = true;
            TimeStartingOffline();
        }
        else // WİFİ BAĞLANTISI BAŞARILIYSA BU FONKSYONLARI YAP.
        {
            wifi_alarm = false;
            FirebaseDeviceIDAssignment();
            timeClient.begin(); // NTP istemcisini başlatma
            TimeStartingOnline();
        }
        first_time_update = true;

        // Görevleri oluştur ve optimize edilmiş yığın boyutlarıyla farklı çekirdeklerde çalıştır
        xTaskCreate(TaskTimeControl, "Time Control Task", 2048, NULL, 7, &TaskTimeControlHandle);               // Zaman kontrolü görevi (2 KB)
        xTaskCreate(TaskFeeder, "Feeder Task", 1536, NULL, 6, &TaskFeederHandle);                               // Yemlik kontrolü görevi (1.5 KB)
        xTaskCreate(TaskSoundControl, "i2s_task", 2048, NULL, 5, &TaskSoundControlHandle);                      // Ses kontrol görevi (2 KB)
        xTaskCreate(TaskServerConnection, "Server Control Task", 8096, NULL, 5, &TaskServerConnectionHandle);   // Sunucu bağlantı görevi (4 KB)
        xTaskCreate(TaskButtonControl, "Button Control Task", 1536, NULL, 4, &TaskButtonControlHandle);         // Buton kontrol görevi (1.5 KB)
        xTaskCreate(TaskFeederFlowControl, "Feeder Control Task", 1536, NULL, 4, &TaskFeederFlowControlHandle); // Yemlik akış kontrol görevi (1.5 KB)
        xTaskCreate(TaskAlarmControl, "Alarm Task", 1024, NULL, 3, &TaskAlarmControlHandle);                    // Alarm kontrol görevi (1 KB)
        xTaskCreate(TaskLightControl, "Light Task", 2048, NULL, 2, &TaskLightControlHandle);                    // Işık kontrol görevi (2 KB)
        xTaskCreate(TaskWaterFeeder, "Water Feeder Task", 1536, NULL, 1, &TaskWaterFeederHandle);               // Su besleyici kontrol görevi (1.5 KB)
        xTaskCreate(TaskMicControl, "Mic Control Task", 1536, NULL, 1, &TaskMicControlHandle);                  // Mikrofon kontrol görevi (1.5 KB)
        xTaskCreate(TaskWaterLevelControl, "Water Control Task", 1536, NULL, 1, &TaskWaterLevelControlHandle);  // Su seviyesi kontrol görevi (1.5 KB)
        xTaskCreate(TaskFoodLevelControl, "Food Control Task", 1536, NULL, 1, &TaskFoodLevelControlHandle);     // Yem seviyesi kontrol görevi (1.5 KB)
        xTaskCreate(TaskSerialPortReport, "Report Task", 2048, NULL, 1, &TaskSerialPortReportHandle);           // Seri port raporlama görevi (2 KB)
        xTaskCreate(TaskBatteryControl, "Battery Control Task", 1024, NULL, 0, &TaskBatteryControlHandle);      // Pil kontrol görevi (1 KB)
        xTaskCreate(TaskOTA, "OTA Task", 8192, NULL, 9, &TaskOTAHandle);                                        // OTA güncelleme görevi (8 KB)

        /*
            // Görevleri oluştur ve farklı çekirdeklerde çalıştır
            xTaskCreate(TaskTimeControl, "Time Control Task", 4096, NULL, 7, &TaskTimeControlHandle);
            xTaskCreate(TaskFeeder, "Feeder Task", 2048, NULL, 6, &TaskFeederHandle);
            xTaskCreate(TaskSoundControl, "i2s_task", configMINIMAL_STACK_SIZE * 5, NULL, 5, &TaskSoundControlHandle);
            xTaskCreate(TaskServerConnection, "Server Control Task", 8096, NULL, 5, &TaskServerConnectionHandle);
            xTaskCreate(TaskButtonControl, "Button Control Task", 2048, NULL, 4, &TaskButtonControlHandle);
            xTaskCreate(TaskFeederFlowControl, "Feeder Control Task", 2048, NULL, 4, &TaskFeederFlowControlHandle);
            xTaskCreate(TaskAlarmControl, "Report Task", 1024, NULL, 3, &TaskAlarmControlHandle);
            xTaskCreate(TaskLightControl, "Light Task", 4096, NULL, 2, &TaskLightControlHandle);
            xTaskCreate(TaskWaterFeeder, "Water Feeder Task", 2048, NULL, 1, &TaskWaterFeederHandle);
            xTaskCreate(TaskMicControl, "Noisy Control Task", 2048, NULL, 1, &TaskMicControlHandle);
            xTaskCreate(TaskWaterLevelControl, "Water Control Task", 2048, NULL, 1, &TaskWaterLevelControlHandle);
            xTaskCreate(TaskFoodLevelControl, "Food Control Task", 2048, NULL, 1, &TaskFoodLevelControlHandle);
            xTaskCreate(TaskSerialPortReport, "Report Task", 4096, NULL, 1, &TaskSerialPortReportHandle);
            xTaskCreate(TaskBatteryControl, "Battery Control Task", 1024, NULL, 0, &TaskBatteryControlHandle);
            xTaskCreate(TaskOTA, "OTA Task", 16384, NULL, 8, &TaskOTAHandle); // Daha büyük bir stack boyutu

            vTaskSuspend(TaskOTAHandle);


            1 KB = 256 kelime (stack size)
            2 KB = 512 kelime (stack size)
            4 KB = 1024 kelime (stack size)
            8 KB = 2048 kelime (stack size)
            */
    }
}
