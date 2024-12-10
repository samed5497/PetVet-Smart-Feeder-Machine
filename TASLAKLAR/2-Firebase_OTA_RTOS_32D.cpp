#include <core/chooser.h>

#ifdef TEST
#include "Arduino.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <HTTPClient.h>
#include <Update.h>

// Firebase ESP Client nesnesi
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// WiFi bilgileri
#define WIFI_SSID "10_Numara"
#define WIFI_PASSWORD "Daire_on10"

// Firebase bilgileri
#define API_KEY "AIzaSyAHQNOtr22VKWIAfQYVHJ9qAl1XkrrB9hI"
#define DATABASE_URL "https://kaya-smart-home-default-rtdb.europe-west1.firebasedatabase.app/"

// Firebase Authentication bilgileri
String email = "kaya.samed@outlook.com";
String email_pass = "SamCo1234";

// Cihazın mevcut versiyonu
#define CURRENT_VERSION "0.5.0"

// Görev tanımlamaları
void TaskOTA(void *parameter);
void TaskSerialPortReport(void *parameter);

// Global değişkenler
bool updating = false;                // Güncelleme durumu
TaskHandle_t serialTaskHandle = NULL; // Seri port raporu görev tanıtıcı

// OTA güncelleme işlemi
void performOTAUpdate(String url)
{
    Serial.println("Firmware indiriliyor: " + url);

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
            if (serialTaskHandle != NULL)
            {
                vTaskSuspend(serialTaskHandle);
            }

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
                    Serial.printf("Yükleme durumu: %d%%\n", percentage);
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

            // Diğer görevleri devam ettir
            updating = false;
            if (serialTaskHandle != NULL)
            {
                vTaskResume(serialTaskHandle);
            }
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

// OTA güncelleme kontrol fonksiyonu
void checkForOTAUpdate()
{
    Serial.println("\n--- OTA Güncelleme Kontrolü Başlıyor ---");

    // Firebase'den versiyon bilgisi alınır
    if (Firebase.RTDB.getString(&fbdo, "UPDATES/PetVet/version"))
    {
        String latestVersion = fbdo.stringData();
        Serial.println("Mevcut Versiyon: " + String(CURRENT_VERSION));
        Serial.println("Sunucudaki Versiyon: " + latestVersion);

        if (latestVersion != CURRENT_VERSION)
        {
            Serial.println("[Info]: Yeni versiyon mevcut! Güncelleme başlatılıyor...");

            // URL bilgisi alınır
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

// Task: OTA güncelleme kontrolü
void TaskOTA(void *parameter)
{
    for (;;)
    {
        if (!updating) // Güncelleme sırasında kontrol yapma
        {
            checkForOTAUpdate();
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 saniye bekle
    }
}

// Task: Seri port raporu
void TaskSerialPortReport(void *parameter)
{
    for (;;)
    {
        if (!updating) // Güncelleme sırasında raporlama yapma
        {
            Serial.printf("Free Heap: %d bytes\n Min Free Heap: %d bytes\n", ESP.getFreeHeap(), ESP.getMinFreeHeap());
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 saniye bekle
    }
}

// setup() ve FreeRTOS görev oluşturma
void setup()
{
    Serial.begin(115200);

    // WiFi bağlantısı
    Serial.println("WiFi'ye bağlanılıyor...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi bağlantısı başarılı!");

    // Firebase yapılandırma
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = email.c_str();
    auth.user.password = email_pass.c_str();

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    if (Firebase.ready())
    {
        Serial.println("Firebase bağlantısı başarılı!");
    }
    else
    {
        Serial.println("Firebase bağlantısı başarısız!");
    }

    // FreeRTOS görevleri oluşturma
    xTaskCreate(TaskOTA, "OTA Update Task", 8192, NULL, 1, NULL);
    xTaskCreate(TaskSerialPortReport, "Serial Report Task", 2048, NULL, 1, &serialTaskHandle);
}

// loop() işlevi boş bırakıldı
void loop()
{
    // FreeRTOS, görevleri bağımsız olarak yönetir
}
#endif
