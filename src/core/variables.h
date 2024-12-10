#include <core/libsandpins.h>

// ****************************************
// VARIABLES ***********************************

Audio audio;
/////////////////////////////////////////////// Wifi değişkenleri

#define WIFI_SSID ssid
#define WIFI_PASSWORD pass

/////////////////////////////////////////////// NTP Sunucu ayarları

const long utcOffsetInSeconds = 10800; // Türkiye için UTC+3
const char *ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

unsigned long currentMillis, previousDayMillis, previousHourMillis, previousMinuteMillis, previousMillis = 0;

bool LocalClockControl = false;

int seconddiffent, minutediffent, hourdiffent, DOWdiffent;
int currentDayofWeekEEPROM, currentYearEEPROM, currentMonthEEPROM, currentDayEEPROM, currentSecondEEPROM, currentMinuteEEPROM, currentHourEEPROM;
int currentYear, currentMonth, currentDay, currentDayofWeek, currentHour, currentMinute, currentSecond;

/////////////////////////////////////////////// EEPROM Değişkenleri

const int EEPROM_SIZE = 4096;

const int HOUR_ADDRESS = 10;
const int MINUTE_ADDRESS = 20;
const int SECOND_ADDRESS = 30;
const int DAY_ADDRESS = 40;
const int MONTH_ADDRESS = 50;
const int YEAR_ADDRESS = 60;
const int DAY_OF_WEEK_ADDRESS = 70;


/////////////////////////////////////////////// Sunucu Değişkenleri

#define USER_EMAIL email
#define USER_PASSWORD email_pass

String uid, VeriYolu;
int id_count = 0;
bool signupOK = false;
int DEVICE_ID = 1;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/////////////////////////////////////////////// Genel değişkenler

int motor_lap_count, wtr_lvl, noisy = 0;
bool switch_durum, feed_mode, EEPROM_Time_Update, first_time_update = false;
bool alarm_status, sikisma_alarm, noisy_alarm, max_water_alarm, min_water_alarm, wifi_alarm = false;
String hata_kodu, latestVersion, firmwareURL = "";

unsigned long buttonPressStartTime = 0;
bool buttonHeld, updating, ManuelUpdateControl = false;
const unsigned long longPressDuration = 1000; // 1 saniye

bool report = false;

/////////////////////////////////////////////// Mikrofon değişkenleri

int noisy_check = 0;
int esikdeger = 1000;

int Speaker_Volume = 0;

/////////////////////////////////////////////// RTC DEĞİŞKEN TANIMLAMALARI

uRTCLib rtc;
byte rtcModel = URTCLIB_MODEL_DS1307;
uint8_t position;
#define RTC_ADDRESS 0x68    // DS1307 RTC adresi
#define EEPROM_ADDRESS 0x50 // 24c32 adresi

/////////////////////////////////////////////// Program değişkenleri

#define PROGRAM_SAYISI 10
String veriAlanlari[] = {"number", "hour", "minute", "portion", "status"};

int feed_program[PROGRAM_SAYISI];
int target_motor_lap_count[PROGRAM_SAYISI];
int feed_time_hour[PROGRAM_SAYISI];
int feed_time_minute[PROGRAM_SAYISI];
bool status[PROGRAM_SAYISI];

int runing_program, last_feed_minute = 0; // vakti gelen görevin id si.
int manuel_feeder_portion = 5;

bool device_server_report, manuel_feeder_status, manuel_water_status, Local_Time_Report, water_timer = false;
bool manuel_feeder_status_report, manuel_water_status_report = true;

int manuel_water_portion = 10; // second
int water_work_time = 10;      // Minute waterrefleshtime
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
TaskHandle_t TaskTimeControlHandle = NULL;
TaskHandle_t TaskFeederHandle = NULL;
TaskHandle_t TaskSoundControlHandle = NULL;
TaskHandle_t TaskServerConnectionHandle = NULL;
TaskHandle_t TaskButtonControlHandle = NULL;
TaskHandle_t TaskFeederFlowControlHandle = NULL;
TaskHandle_t TaskAlarmControlHandle = NULL;
TaskHandle_t TaskLightControlHandle = NULL;
TaskHandle_t TaskWaterFeederHandle = NULL;
TaskHandle_t TaskMicControlHandle = NULL;
TaskHandle_t TaskWaterLevelControlHandle = NULL;
TaskHandle_t TaskFoodLevelControlHandle = NULL;
TaskHandle_t TaskSerialPortReportHandle = NULL;
TaskHandle_t TaskBatteryControlHandle = NULL;
TaskHandle_t TaskOTAHandle = NULL;

void TaskOTA(void *pvParameters);
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
