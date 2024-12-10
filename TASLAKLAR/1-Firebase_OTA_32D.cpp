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
        if (contentLength > 0)
        {
            Serial.printf("Firmware boyutu: %.2f KB\n", contentLength / 1024.0);

            if (Update.begin(contentLength))
            {
                size_t written = 0;
                size_t prevPercentage = 0;

                while (written < contentLength)
                {
                    size_t chunk = client->available();
                    if (chunk)
                    {
                        uint8_t buffer[128];
                        size_t bytesRead = client->readBytes(buffer, min(sizeof(buffer), chunk));
                        written += Update.write(buffer, bytesRead);

                        // Yüzde hesaplama
                        size_t percentage = (written * 100) / contentLength;
                        if (percentage - prevPercentage >= 5) // %5'lik artış olduğunda yazdır
                        {
                            Serial.printf("İlerleme: %d%% (%.2f KB / %.2f KB)\n", percentage, written / 1024.0, contentLength / 1024.0);
                            prevPercentage = percentage;
                        }
                    }
                    delay(1); // İşlemi kesintisiz devam ettirmek için kısa bir gecikme
                }

                if (written == contentLength)
                {
                    Serial.println();
                    Serial.println("[Info]: OTA başarılı!");
                    if (Update.end(true))
                    {
                        Serial.println("[Info]: Cihaz yeniden başlatılıyor...");
                        ESP.restart();
                    }
                    else
                    {
                        Serial.printf("[ERROR]: OTA doğrulama hatası: %s\n", Update.errorString());
                    }
                }
                else
                {
                    Serial.println("[ERROR]: OTA yazma hatası!");
                }
            }
            else
            {
                Serial.printf("[ERROR]: OTA başlangıç hatası: %s\n", Update.errorString());
            }
        }
        else
        {
            Serial.println("[ERROR]: Geçersiz firmware boyutu!");
        }
    }
    else
    {
        Serial.printf("[ERROR]: HTTP Hatası: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}




void setup()
{
    Serial.begin(115200);

    Serial.println("WiFi'ye bağlanılıyor...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi bağlantısı başarılı!");

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

    Serial.printf("Total Flash Size: %u KB\n", ESP.getFlashChipSize() / 1024);
    Serial.printf("Used Flash: %u KB\n", ESP.getSketchSize() / 1024);
    Serial.printf("Free Flash: %u KB\n", (ESP.getFlashChipSize() - ESP.getSketchSize()) / 1024);
}

void loop()
{
    delay(6000); 
    checkForOTAUpdate();
}
#endif
