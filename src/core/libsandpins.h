
// ****************************************
// LIBS ***********************************

#include "WiFi.h"
#include "Audio.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <ArduinoJson.h>
#include "uRTCLib.h"

#include <PETVET_config.h>

// ****************************************
// Core Variables *************************

String UYGULAMA = "PetVet ";
String VERSIYON = "v0.2_beta-by S.KAYA";
const char *hostname = "PetVet";

// ****************************************
// PINS ***********************************

#define mic_pin 39  // 4 ten taşındı
#define batt_pin 36 // 5 ten taşındı
#define buzzer_pin 18
#define neo_pin 19
#define button_pin 17 // 35 ten taşındı
#define button2_pin 0
#define weter_level_pin 35
#define switch_pin 34
#define I2C_gnd 33

#define m1_en_pin 32 // su motoru
#define m1_in1_pin 12
#define m1_in2_pin 14

#define m2_en_pin 15 // feder motoru
#define m2_in1_pin 16
#define m2_in2_pin 13

#define I2C_SDA_pin 21
#define I2C_SCL_pin 22
#define I2C_VL53L0X_en_pin 33

#define I2S_DIN_pin 25   //
#define I2S_LRCLK_pin 26 // MAX98357AETE
#define I2S_BCLK_pin 27  //

void pins()
{
    pinMode(buzzer_pin, OUTPUT);
    pinMode(m1_en_pin, OUTPUT);
    pinMode(m1_in1_pin, OUTPUT);
    pinMode(m1_in2_pin, OUTPUT);
    pinMode(m2_en_pin, OUTPUT);
    pinMode(m2_in1_pin, OUTPUT);
    pinMode(m2_in2_pin, OUTPUT);
    pinMode(I2C_gnd, OUTPUT);

    pinMode(mic_pin, INPUT);
    pinMode(batt_pin, INPUT);
    pinMode(button_pin, INPUT_PULLUP);
    pinMode(button2_pin, INPUT);
    pinMode(weter_level_pin, INPUT);
    pinMode(switch_pin, INPUT);
}

// ****************************************
// OTHERS *********************************

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();