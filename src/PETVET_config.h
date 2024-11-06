#include <Arduino.h>
#include <core/chooser.h>

#if defined(PetVet) || defined(PetVetVeriGonder)

/*
/////////////////////////////////////////////// Wifi değişkenleri

String ssid = "XXXXXXXXXX";
String pass = "XXXXXXXXXX";

/////////////////////////////////////////////// Sunucu Değişkenleri

String email = "XXXXXXX@XXXXXXXX.com";
String email_pass = "XXXXXXXXXXXX";

#define API_KEY "XXXXXXXXXXXXXXXX"
#define DATABASE_URL "https://XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.firebasedatabase.app/"
*/

/////////////////////////////////////////////// Wifi değişkenleri

String ssid = "10_Numara";
String pass = "Daire_on10";

/////////////////////////////////////////////// Sunucu Değişkenleri

String email = "kaya.samed@outlook.com";
String email_pass = "SamCo1234";

#define API_KEY "AIzaSyBEet3NVwQrrLnu8ZuDE5NpHvtEcAnLdqY"
#define DATABASE_URL "https://petvet-001-default-rtdb.europe-west1.firebasedatabase.app/"

/////////////////////////////////////////////// Seriport Değişkenleri

bool report_mode = true;

/////////////////////////////////////////////// Zaman Sağlayıcı Mod

// #define WiFi_clock
#define WiFi_and_RTC_clock
// #define RTC_clock

#if defined(RTC_clock)
#define currentSecond __TIME__[6] + __TIME__[7] // Şu anki saniye
#define currentMinute __TIME__[3] + __TIME__[4] // Şu anki dakika
#define currentHour __TIME__[0] + __TIME__[1]   // Şu anki saat
#define currentDayofWeek __DATE__               // Haftanın günü, ay ve gün bilgisi
#endif

#endif
