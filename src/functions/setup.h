#include <functions/aux_funcs.h>

void WifiStarting()
{
    Serial.printf("\r\n[Wifi]: Bağlantı için bekleniyor..");

    WiFi.mode(WIFI_STA); // STA MODUNU devre dışı bırak.
    WiFi.disconnect();
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.hostname(hostname);
    WiFi.setHostname(hostname); // ESP32 için hostname ayarı

    WiFi.begin(ssid, pass);
    int count = 0;
    while (count < 30)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println();
            Serial.printf("[WiFi]: Bağlantı Başarılı!\r\n[WiFi]: Yerel IP-Addresi: %s\r\n", WiFi.localIP().toString().c_str());
            Serial.printf("[WiFi]: MAC Addresi     : %s\r\n", WiFi.macAddress().c_str());

            break;
        }

        Serial.print(".");
        delay(250);
        colorWipe(strip.Color(255, 255, 255), 0);
        Serial.print(".");
        delay(250);
        colorWipe(strip.Color(0, 0, 0), 0);
        count++;
    }
    Serial.println();
}

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
                    Serial.println("[Info]: Mevcut cihaz bulundu.");
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

            // Settings bilgilerini ekle
            json.set("/settings/serialport", report_mode);
            json.set("/settings/soundvolume", Speaker_Volume);

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
                json.set(programPath + "/portion", 0);
                json.set(programPath + "/status", 0);
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
                Serial.println(fbdo.errorReason());
                return;
            }
        }

        // Cihaz ID'sini belirle
        DEVICE_ID = DEVICE_ID;
        Serial.printf("[Info]: Cihaz ID: DEVICE%d olarak ayarlandı.\n", DEVICE_ID);
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
                Serial.print("[ERROR]: Mail güncellenemedi. Nedeni: ");
                Serial.println(fbdo.errorReason());
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
                Serial.print("[ERROR]: Mail oluşturulamadı. Nedeni: ");
                Serial.println(fbdo.errorReason());
            }
        }
        else
        {
            Serial.print("[ERROR]: Mail kontrolü başarısız. Nedeni: ");
            Serial.println(fbdo.errorReason());
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
                Serial.print("[ERROR]: Mail şifresi güncellenemedi. Nedeni: ");
                Serial.println(fbdo.errorReason());
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
                Serial.print("[ERROR]: Mail şifresi oluşturulamadı. Nedeni: ");
                Serial.println(fbdo.errorReason());
            }
        }
        else
        {
            Serial.print("[ERROR]: Mail şifre kontrolü başarısız. Nedeni: ");
            Serial.println(fbdo.errorReason());
        }
    }
}

void FirebaseServerStarting()
{
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = email;
    auth.user.password = email_pass;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;
    Firebase.begin(&config, &auth);

    Serial.println("");
    Serial.println("[Info]: Kullanıcı bilgileri doğrulanıyor..");

    while ((auth.token.uid) == "" && id_count < 75)
    {
        Serial.print('.');
        vTaskDelay(250);
        id_count++;
    }
    if ((auth.token.uid) == "")
    {
        Serial.println("[***ERROR!] - Zaman aşımına uğrandı.");
        Serial.println();
        vTaskDelay(500);
    }
    else
    {
        uid = auth.token.uid.c_str();
        signupOK = true;
        Serial.print("[Info]: Doğrulanan Kimlik ID: ");
        Serial.println(uid);

        if (Firebase.ready() && signupOK)
        {
            FirebaseDeviceIDAssignment();
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
    // [BUILDING] = Buraya eepromdan okudugu degeri timeclient sunucusunun varsayılan başlatma ayarı olarak ata.
    // timeClient.update();
    currentDayofWeek = EEPROMRead(DAY_OF_WEEK_ADDRESS);
    currentYear = EEPROMRead(YEAR_ADDRESS);
    currentMonth = EEPROMRead(MONTH_ADDRESS);
    currentDay = EEPROMRead(DAY_ADDRESS);
    currentSecond = EEPROMRead(SECOND_ADDRESS);
    currentMinute = EEPROMRead(MINUTE_ADDRESS);
    currentHour = EEPROMRead(HOUR_ADDRESS);
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

        timeClient.update();

        currentDayofWeek = timeClient.getDay();
        currentYear = EEPROMRead(60);
        currentMonth = EEPROMRead(50);
        currentDay = EEPROMRead(40);
        currentSecond = timeClient.getSeconds();
        currentMinute = timeClient.getMinutes();
        currentHour = timeClient.getHours();

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
}

// ****************************************
// SETUP ***********************************

void setup()
{
    pins();                    // Pin Tanımlamaları
    StartingSerialandApps();   // İlk çalıştırılan uygulama ve seri port mesajları
    EEPROM.begin(EEPROM_SIZE); // EEPROM'i başlat
    WifiStarting();            // Wifi Başlatma Uygulamalrı
    AmplificatorStarting();    // MAX98 Entegresini devreye al

    /////////////////////////////////////////////// Başlangıç Uygulamarı Sonrası İlk Fonksiyonlar

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

        timeClient.begin(); // NTP istemcisini başlatma
        TimeStartingOnline();
        FirebaseServerStarting(); // Firebase Başlatma Uygulamalrı
    }
    first_time_update = true;

    // Görevleri oluştur ve farklı çekirdeklerde çalıştır
    xTaskCreate(TaskTimeControl, "Time Control Task", 4096, NULL, 7, NULL);
    xTaskCreate(TaskFeeder, "Feeder Task", 2048, NULL, 6, NULL);
    xTaskCreate(TaskSoundControl, "i2s_task", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    xTaskCreate(TaskServerConnection, "Server Control Task", 8096, NULL, 5, NULL);
    xTaskCreate(TaskButtonControl, "Button Control Task", 2048, NULL, 4, NULL);
    xTaskCreate(TaskFeederFlowControl, "Feeder Control Task", 2048, NULL, 4, NULL);
    xTaskCreate(TaskAlarmControl, "Report Task", 1024, NULL, 3, NULL);
    xTaskCreate(TaskLightControl, "Ligth Task", 4096, NULL, 2, NULL);
    xTaskCreate(TaskWaterFeeder, "Water Feeder Task", 2048, NULL, 1, NULL);
    xTaskCreate(TaskMicControl, "Noisy Control Task", 2048, NULL, 1, NULL);
    xTaskCreate(TaskWaterLevelControl, "Water Control Task", 2048, NULL, 1, NULL);
    xTaskCreate(TaskFoodLevelControl, "Food Control Task", 2048, NULL, 1, NULL);
    xTaskCreate(TaskSerialPortReport, "Report Task", 4096, NULL, 1, NULL);
    xTaskCreate(TaskBatteryControl, "Battery Control Task", 1024, NULL, 0, NULL);
    xTaskCreate(TaskOTA, "ota Task", 16384, NULL, 8, NULL);

    /*
    1 KB = 256 kelime (stack size)
    2 KB = 512 kelime (stack size)
    4 KB = 1024 kelime (stack size)
    8 KB = 2048 kelime (stack size)
    */
}
