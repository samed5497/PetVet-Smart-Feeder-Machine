
#include <core/chooser.h>

#ifdef PetVetVeriGonder
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ArduinoJson.h>

#include <PETVET_config.h>

/////////////////////////////////////////////// Wifi değişkenleri

#define WIFI_SSID ssid
#define WIFI_PASSWORD pass

/////////////////////////////////////////////// Sunucu Değişkenleri

#define USER_EMAIL email
#define USER_PASSWORD email_pass

String uid, VeriYolu;
int id_count = 0;
bool signupOK = false;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/////////////////////////////////////////////// XXXX Değişkenleri

#define PROGRAM_SAYISI 10
int feed_program[PROGRAM_SAYISI];
int target_motor_lap_count[PROGRAM_SAYISI];
int feed_time_hour[PROGRAM_SAYISI];
int feed_time_minute[PROGRAM_SAYISI];
bool status[PROGRAM_SAYISI];

/////////////////////////////////////////////// XXXX Değişkenleri

void Wifi_Starting()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
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
        delay(250);
        id_count++;
    }
    if ((auth.token.uid) == "")
    {
        Serial.println("***HATA! - Zaman aşımına uğrandı.");
        Serial.println();
        delay(500);
    }
    else
    {
        uid = auth.token.uid.c_str();
        signupOK = true;
        Serial.print("Bilgi- Doğrulanan Kimlik ID: ");
        Serial.println(uid);

        if (Firebase.ready() && signupOK)
        {
            VeriYolu = uid + "/mail";
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, USER_EMAIL))
            {
                Serial.println("[Info]: Mail sended.");
            }
            else
            {
                Serial.print("[ERROR]: Connection FAILED. ");
                Serial.println("REASON: " + fbdo.errorReason());
            }

            VeriYolu = uid + "/macadress";
            if (Firebase.RTDB.setString(&fbdo, VeriYolu, WiFi.macAddress()))
            {
                Serial.println("[Info]: MAC Adress sended.");
            }
            else
            {
                Serial.print("[ERROR]: Connection FAILED. ");
                Serial.println("REASON: " + fbdo.errorReason());
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);

    Wifi_Starting();
    Firebase_Server_Starting();
}

void loop()
{
    if (Firebase.ready() && signupOK)
    {
        /*
         VeriYolu = uid + "/programs/oto/program1/hour";
         if (Firebase.RTDB.setInt(&fbdo, VeriYolu, 5))
         {
             Serial.println("PASSED setInt");
         }
         else
         {
             Serial.print("[ERROR]: Connection FAILED. ");
             Serial.println("REASON: " + fbdo.errorReason());
         }
   */
        // Firebase'den veriyi çekme ve ayrıştırma

        VeriYolu = uid + "/programs/oto/";

        if (Firebase.RTDB.getJSON(&fbdo, VeriYolu))
        {
            Serial.println("veri alındı");

            DynamicJsonDocument doc(2048);
            deserializeJson(doc, fbdo.jsonString());

            for (int i = 1; i <= PROGRAM_SAYISI; i++)
            {
                String programKey = "program" + String(i);

                if (doc.containsKey(programKey))
                {
                    feed_program[i - 1] = doc[programKey]["number"];
                    feed_time_hour[i - 1] = doc[programKey]["hour"];
                    feed_time_minute[i - 1] = doc[programKey]["minute"];
                    target_motor_lap_count[i - 1] = doc[programKey]["portion"];
                    status[i - 1] = doc[programKey]["status"];
                }
            }
        }
        else
        {
            Serial.print("[ERROR]: Connection FAILED. ");
            Serial.println("REASON: " + fbdo.errorReason());
        }

        // Çekilen verileri seri portta göster
        for (int i = 0; i < PROGRAM_SAYISI; i++)
        {
            Serial.print("Program ");
            Serial.println(i + 1);
            Serial.print("Number: ");
            Serial.println(feed_program[i]);
            Serial.print("Hour: ");
            Serial.println(feed_time_hour[i]);
            Serial.print("Minute: ");
            Serial.println(feed_time_minute[i]);
            Serial.print("Portion: ");
            Serial.println(target_motor_lap_count[i]);
            Serial.print("Status: ");
            Serial.println(status[i]);
            Serial.println();
        }
    }
    delay(10000);
}

#endif
