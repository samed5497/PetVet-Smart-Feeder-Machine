#include <functions/aux_funcs.h>

void Wifi_Starting()
{
    Serial.printf("\r\n[Wifi]: Bağlantı için bekleniyor..");

    WiFi.mode(WIFI_STA); // STA MODUNU devre dışı bırak.
    WiFi.disconnect();
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(hostname);
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
        vTaskDelay (250);
        colorWipe(strip.Color(255, 255, 255), 0);
        Serial.print(".");
        vTaskDelay (250);
        colorWipe(strip.Color(0, 0, 0), 0);
        count++;
    }
    Serial.println();
}

void Firebase_Server_Starting()
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
    Serial.println("Bilgi- Kullanıcı bilgileri doğrulanıyor..");

    while ((auth.token.uid) == "" && id_count < 75)
    {
        Serial.print('.');
        vTaskDelay (250);
        id_count++;
    }
    if ((auth.token.uid) == "")
    {
        Serial.println("***HATA! - Zaman aşımına uğrandı.");
        Serial.println();
        vTaskDelay (500);
    }
    else
    {
        uid = auth.token.uid.c_str();
        signupOK = true;
        Serial.print("[Info]: Doğrulanan Kimlik ID: ");
        Serial.println(uid);

        if (Firebase.ready() && signupOK)
        {
            VeriYolu.clear();
            VeriYolu.concat(uid);
            VeriYolu.concat("/device/mail");
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, USER_EMAIL))
            {
                Serial.println("[Info]: Mail sended.");
            }
            else
            {
                Serial.print("[ERROR]: Connection FAILED. ");
                Serial.println("REASON: " + fbdo.errorReason());
            }
            VeriYolu.clear();
            VeriYolu.concat(uid);
            VeriYolu.concat("/device/macadress");
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, WiFi.macAddress()))
            {
                Serial.println("[Info]: MAC Adress sended.");
            }
            else
            {
                Serial.print("[ERROR]: Connection FAILED. ");
                Serial.println("REASON: " + fbdo.errorReason());
            }
            VeriYolu.clear();
            VeriYolu.concat(uid);
            VeriYolu.concat("/device/ipadress");
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, WiFi.localIP().toString().c_str()))
            {
                Serial.println("[Info]: IP Adress sended.");
            }
            else
            {
                Serial.print("[ERROR]: Connection FAILED. ");
                Serial.println("REASON: " + fbdo.errorReason());
            }
        }
    }
}

void Amplificator_Starting()
{
    audio.setPinout(I2S_BCLK_pin, I2S_LRCLK_pin, I2S_DIN_pin);
    audio.setVolume(map(Speaker_Volume, 0, 100, 0, 21));                // 0...21
    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
}

void RTC_Starting_Offline()
{
    wifi_alarm = true;
    Wire.begin();
    URTCLIB_WIRE.begin();
    rtc.set_rtc_address(RTC_ADDRESS);
    rtc.set_model(rtcModel);
    rtc.refresh();

    // Epprom_Update_From_PC(); // İlk yuklemelerde çalıştırılacak.

    currentDayofWeek = readEEPROM(70);
    currentYear = readEEPROM(60);
    currentMount = readEEPROM(50);
    CurrentDay = readEEPROM(40);
    currentSecond = readEEPROM(30);
    currentMinute = readEEPROM(20);
    currentHour = readEEPROM(10);
    Serial.println("[Info]: Cihaz saati Offline olarak ayarlandı.");

    //
    //
    // Yalnızca bir kez kullanın, ardından devre dışı bırakın
    rtc.set(currentSecond, currentMinute, currentHour, currentDayofWeek, CurrentDay, currentMount, currentYear);
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

void RTC_Starting_Online()
{
    Wire.begin();
    URTCLIB_WIRE.begin();
    rtc.set_rtc_address(RTC_ADDRESS);
    rtc.set_model(rtcModel);
    rtc.refresh();

    // Epprom_Update_From_PC();

    timeClient.update();

    currentDayofWeek = timeClient.getDay();
    currentYear = readEEPROM(60);
    currentMount = readEEPROM(50);
    CurrentDay = readEEPROM(40);
    currentSecond = timeClient.getSeconds();
    currentMinute = timeClient.getMinutes();
    currentHour = timeClient.getHours();

    //
    //
    // Yalnızca bir kez kullanın, ardından devre dışı bırakın
    rtc.set(currentSecond, currentMinute, currentHour, currentDayofWeek, CurrentDay, currentMount, currentYear);
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

void Starting_SerialandApps()
{
    Serial.begin(115200);
    Serial.println("HMI Başlatılıyor. ");
    Serial.println();
    Serial.println();
    Serial.print(" >>> ");
    Serial.print(UYGULAMA);
    Serial.print(" / ");
    Serial.println(VERSIYON);
    Serial.printf("\r\n");
    vTaskDelay (10);
    Serial.flush();
    digitalWrite(buzzer_pin, HIGH);
    vTaskDelay (250);
    digitalWrite(buzzer_pin, LOW);
    tone(buzzer_pin, 1000); // 1000 Hz frekansta ses üret
    vTaskDelay (250);             // 250 milisaniye bekle
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
    Starting_SerialandApps();  // İlk çalıştırılan uygulama ve seri port mesajları
    EEPROM.begin(EEPROM_SIZE); // EEPROM'i başlat

    Wifi_Starting(); // Wifi Başlatma Uygulamalrı

    /////////////////////////////////////////////// Başlangıç Uygulamarı Sonrası İlk Fonksiyonlar

    if (WiFi.status() != WL_CONNECTED) // WİFİ BAĞLANTISI OLMADIYSA BU FONKSYONLARI YAP.
    {
        Serial.println();
        Serial.println("[ALARM]: WiFi connection failed, using EEPROM time");
        Serial.println();

        wifi_alarm = true;

        RTC_Starting_Offline();
    }
    else // WİFİ BAĞLANTISI BAŞARILIYSA BU FONKSYONLARI YAP.
    {
        Amplificator_Starting(); // MAX98 Entegresini devreye al
        timeClient.begin();      // NTP istemcisini başlatma
        first_time_update = true;
        Firebase_Server_Starting(); // Firebase Başlatma Uygulamalrı
        RTC_Starting_Online();
        timeClient.end(); // NTP istemcisini BİTİRME
    }

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

    /*
    1 KB = 256 kelime (stack size)
    2 KB = 512 kelime (stack size)
    4 KB = 1024 kelime (stack size)
    8 KB = 2048 kelime (stack size)
    */
}
