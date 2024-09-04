#include <core/libsandpins.h>

// ****************************************
// VARIABLES ***********************************

Audio audio;
/////////////////////////////////////////////// Wifi değişkenleri

String ssid = "";
String pass = "";

#define WIFI_SSID ssid
#define WIFI_PASSWORD pass

/////////////////////////////////////////////// NTP Sunucu ayarları

const long utcOffsetInSeconds = 10800; // Türkiye için UTC+3
const char *ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

unsigned long previousDayMillis, previousHourMillis, previousMinuteMillis, previousMillis, currentMillis = 0;

const int EEPROM_SIZE = 64;

const int DAY_ADDRESS = 0;
const int HOUR_ADDRESS = 4;
const int MINUTE_ADDRESS = 8;
const int SECOND_ADDRESS = 12;

int currentDayofWeek, currentYear, currentMount, CurrentDay, currentHour, currentMinute, currentSecond;

/////////////////////////////////////////////// Sunucu Değişkenleri

String email = "";
String email_pass = "";

String uid, VeriYolu;
int id_count = 0;
bool signupOK = false;

#define API_KEY ""
#define DATABASE_URL ""
#define USER_EMAIL email
#define USER_PASSWORD email_pass

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/////////////////////////////////////////////// Genel değişkenler

int motor_lap_count,
    wtr_lvl, noisy = 0;
bool switch_durum, feed_mode, Time_Update, first_time_update = false;
bool alarm_status, sikisma_alarm, noisy_alarm, max_water_alarm, min_water_alarm, wifi_alarm = false;
bool report_mode = true;
String hata_kodu = "";

unsigned long buttonPressStartTime = 0;
bool buttonHeld = false;
const unsigned long longPressDuration = 1000; // 1 saniye

bool report = false;

/////////////////////////////////////////////// Mikrofon değişkenleri

int noisy_check = 0;
int esikdeger = 1000;

int Speaker_Volume = 100;

/////////////////////////////////////////////// RTC DEĞİŞKEN TANIMLAMALARI

uRTCLib rtc;
byte rtcModel = URTCLIB_MODEL_DS1307;
uint8_t position;
#define RTC_ADDRESS 0x68    // DS1307 RTC adresi
#define EEPROM_ADDRESS 0x50 // 24c32 adresi

/////////////////////////////////////////////// Program değişkenleri

#define PROGRAM_SAYISI 10
int feed_program[PROGRAM_SAYISI];
int target_motor_lap_count[PROGRAM_SAYISI];
int feed_time_hour[PROGRAM_SAYISI];
int feed_time_minute[PROGRAM_SAYISI];
bool status[PROGRAM_SAYISI];

int runing_program, last_feed_minute = 0; // vakti gelen görevin id si.
int manuel_feeder_portion = 5;

bool device_server_report, manuel_feeder_status, manuel_water_status, Local_Time_Report, water_timer = false;
bool manuel_feeder_status_report, manuel_water_status_report =true;

int manuel_water_portion = 10; // second
int water_work_time = 1;       // Minute
unsigned long last_water_time = 0;

unsigned long ilk_temas, son_temas, son_tiklama_zamani = 0;

bool control = false;
unsigned long sayac = 0;

int Water_Level = 9;
int Food_Level = 9;

int SERVER_Food_Level, SERVER_Water_Level, SERVER_batterylevel = 0;
String SERVER_last_manuel_mod_feed_time, last_manuel_mod_feed_time = "";

/////////////////////////////////////////////// Batarya Değişkenler

float V_Battery, batterylevel = 0;
bool charge_mod = false;

/////////////////////////////////////////////// WS2812B Led değişkenleri

#define PIN neo_pin
Adafruit_NeoPixel strip = Adafruit_NeoPixel(5, PIN, NEO_GRB + NEO_KHZ800);
int Led_Parlaklik = 255;
int LED_R, LED_G, LED_B, Led_Renk = 0;

/////////////////////////////////////////////// Sistem değişkenleri

int CPU_Temperature, last_boot_sure = 0;

// ****************************************
// TASKS ***********************************

void TaskServerConnection(void *pvParameters);
void TaskButtonControl(void *pvParameters);
void TaskWaterFeeder(void *pvParameters);
void TaskFeeder(void *pvParameters);
void TaskFeederFlowControl(void *pvParameters);
void TaskSoundControl(void *pvParameters);
void TaskBatteryControl(void *pvParameters);
void TaskMicControl(void *pvParameters);
void TaskWaterLevelControl(void *pvParameters);
void TaskSerialPortReport(void *pvParameters);
void TaskAlarmControl(void *pvParameters);
void TaskTimeControl(void *pvParameters);
void TaskLightControl(void *pvParameters);
void TaskFoodLevelControl(void *pvParameters);
