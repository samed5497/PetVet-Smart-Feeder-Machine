#include <Arduino.h>
#include <core/chooser.h>

#if defined(PetVet)

String UYGULAMA = "PetVet ";
#define CURRENT_VERSION VERSION
/////////////////////////////////////////////// Wifi değişkenleri
/*
String ssid_STA = "XXXXXXXXXX";
String pass_STA = "XXXXXXXXXX";

/////////////////////////////////////////////// Sunucu Değişkenleri

String email = "XXXXXXX@XXXXXXXX.com";
String email_pass = "XXXXXXXXXXXX";

#define API_KEY "XXXXXXXXXXXXXXXX"
#define DATABASE_URL "https://XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.firebasedatabase.app/"
*/

#define API_KEY "AIzaSyAHQNOtr22VKWIAfQYVHJ9qAl1XkrrB9hI"
#define DATABASE_URL "https://kaya-smart-home-default-rtdb.europe-west1.firebasedatabase.app/"

/////////////////////////////////////////////// Seriport Değişkenleri

bool report_mode = false;

/////////////////////////////////////////////// Bluetooth Değişkenleri

bool BluetoothMode = false;

/////////////////////////////////////////////// Zaman Sağlayıcı Mod

// #define WiFi_clock
#define WiFi_and_RTC_clock

#endif
