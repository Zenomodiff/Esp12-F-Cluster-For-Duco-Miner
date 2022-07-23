#include <ArduinoJson.h> 
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char *ssid = "Code47"; 
const char *password = "Code47@123"; 
const String ducoUser = "iro_d";

const String ducoReportJsonUrl = "https://server.duinocoin.com/v2/users/" + ducoUser + "?limit=1";
const int run_in_ms = 3000;
float lastAverageHash = 0.0;
float lastTotalHash = 0.0;

void setup() {
  
  Serial.begin(115200);
  lcd.begin();          
  lcd.backlight();
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  lcd.setCursor(1,1);
  lcd.print("Duino Miner Stats"); 
  delay(2000);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  
  if (runEvery(run_in_ms)) {

    float totalHashrate = 0.0;
    float avgHashrate = 0.0;
    int total_miner = 0;
    String input = httpGetString(ducoReportJsonUrl);

    DynamicJsonDocument doc (8192);
    DeserializationError error = deserializeJson(doc, input);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject result = doc["result"];
    JsonObject result_balance = result["balance"];
    double result_balance_balance = result_balance["balance"];
    double result_balance_stake = result_balance["stake_amount"];
    const char *result_balance_created = result_balance["created"];
    const char *result_balance_username = result_balance["username"];
    const char *result_balance_verified = result_balance["verified"];

    for (JsonObject result_miner : result["miners"].as<JsonArray>()) {
      float result_miner_hashrate = result_miner["hashrate"];
      totalHashrate = totalHashrate + result_miner_hashrate;
      total_miner++;
    }

    avgHashrate = totalHashrate / long(total_miner);
    long run_span = run_in_ms / 1000;

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Balance :"); 
  lcd.setCursor(10,0);
  lcd.print(result_balance_balance); 
  lcd.setCursor(0,1);
  lcd.print("Stake   :"); 
  lcd.setCursor(10,1);
  lcd.print(result_balance_stake); 
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Workers :"); 
  lcd.setCursor(10,0);
  lcd.print(total_miner); 
  lcd.setCursor(0,1);
  lcd.print("Hash    :"); 
  lcd.setCursor(10,1);
  lcd.print(totalHashrate);

    Serial.println("Username : " + String(result_balance_username));
    Serial.println("Balance : " + String(result_balance_balance));
    Serial.println("Stake : " + String(result_balance_stake));
    Serial.println("Total Hashrate : " + String(totalHashrate));
    Serial.println("Avg Hashrate H/s : " + String(avgHashrate));
    Serial.println("Total Miner : " + String(total_miner));
  }
}

String httpGetString(String URL) {

  String payload = "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (http.begin(client, URL)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  return payload;
}
boolean runEvery(unsigned long interval) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
