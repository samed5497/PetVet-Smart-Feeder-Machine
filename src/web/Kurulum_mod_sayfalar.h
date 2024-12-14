#include <web/Web_Server_Fonksiyonlar.h>

void Kurulum_mod_sayfalar()
{
  Serial.println("[Yerel Sunucu]: Bağlantı başlatılıyor.");
  //////////////settings  //////////////////////////////////////////////////////////////////////////////////////////////////////////////

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String s = generateSettingsForm();

        request->send(200, "text/html", s); });

  /////////////setap      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  server.on("/setap", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    Serial.println("[EEPROM Info]: Eeprom Sıfırlanıyor. ");

    for (int i = 0; i < EEPROM_SIZE; ++i) // Tüm hafıza öncelikle sıfırlanıyor. 
    {
      EEPROM.write(i, 0);
    }

    if (request->hasParam("ssid_STA", true) &&
        request->hasParam("pass_STA", true) &&
        request->hasParam("email", true) &&
        request->hasParam("email_pass", true))
    {
      String ssid_STA = request->getParam("ssid_STA", true)->value();
      String pass_STA = request->getParam("pass_STA", true)->value();
      String email = request->getParam("email", true)->value();
      String email_pass = request->getParam("email_pass", true)->value();

      Serial.printf("  SSID Adı (%d) : %s\n", ssid_STA.length(), ssid_STA.c_str());
      Serial.printf("  Şifre (%d) : %s\n", pass_STA.length(), pass_STA.c_str());
      Serial.printf("  Email (%d) : %s\n", email.length(), email.c_str());
      Serial.printf("  Şifre (%d) : %s\n", email_pass.length(), email_pass.c_str());

      Serial.println("[EEPROM Info]: Bilgiler kaydediliyor...");

      // EEPROM'a yaz
      int ssidLen = ssid_STA.length();
      int passLen = pass_STA.length();
      int emailLen = email.length();
      int email_passLen = email_pass.length();

      for (int i = 0; i < ssidLen; ++i) {
        EEPROM.write(WIFI_SSID_ADDRESS+i, ssid_STA[i]);
      }

      for (int i = 0; i < passLen; ++i) {
        EEPROM.write(WIFI_PASS_ADDRESS + i, pass_STA[i]);
      }

      for (int i = 0; i < emailLen; ++i)
      {
        EEPROM.write(WIFI_MAIL_ADDRESS + i, email[i]);
      }

      for (int i = 0; i < email_passLen; ++i)
      {
        EEPROM.write(WIFI_MAIL_PASS_ADDRESS + i, email_pass[i]);
      }

      EEPROM.commit();
      Serial.println("[EEPROM Info]: Kayıt işlemi başarılı!");

      String s = generateSettingsFinishForm();
                     request->send(200, "text/html", s);
      ESP.restart();
    } 
    else 
    {
      request->send(400, "text/plain", "Eksik Bilgiler Mevcut!");
    } });

  /////////////onNotFound   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    String s = generateFirstPageForm();
    request->send(200, "text/html", s); });
}