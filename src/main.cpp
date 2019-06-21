#include "DHT.h"
#include "properties.h"
#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

#define TELEGRAM_UPDATE_TIME 1000
#define PIN_CONNECTED 12
#define PIN_CONNECTING 14
#define PIN_DHT 04
#define PIN_GATE 5
#define TYPE_DHT DHT22


const char ssid[] = WIFI_SSID;
const char pass[] = WIFI_PASS;
const char botToken[] = BOT_TOKEN;

WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);
DHT dht(PIN_DHT, TYPE_DHT);

#define TELEGRAM_UPDATE_TIME 1000
unsigned long telegramLastUpdateTime;

void connectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(PIN_CONNECTING, HIGH);
    digitalWrite(PIN_CONNECTED, LOW);

    WiFi.disconnect();
    Serial.print("Connecting Wifi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }

    digitalWrite(PIN_CONNECTED, HIGH);
    digitalWrite(PIN_CONNECTING, LOW);

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void setupPins() {
  pinMode(PIN_CONNECTED, OUTPUT);
  pinMode(PIN_CONNECTING, OUTPUT);
  pinMode(PIN_GATE, OUTPUT);
  digitalWrite(PIN_GATE,LOW);
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  delay(1000);
  client.setInsecure();
  setupPins();
  dht.begin();
  connectWiFi();
  telegramLastUpdateTime = millis();
}

void readTemperature(String chat_id) {
  float t = dht.readTemperature();
  char result[30];
  sprintf(result, "A temperatura atual é %.2f C", t);
  Serial.println(result);
  bot.sendMessage(chat_id, result, "");
  Serial.println("Mensagem enviada");
}

void readHumidity(String chat_id) {
  float t = dht.readHumidity();
  char result[30];
  sprintf(result, "A umidade atual é %.2f", t);
  Serial.println(result);
  bot.sendMessage(chat_id, result, "");
  Serial.println("Mensagem enviada");
}


void doGate(String chat_id) {
  digitalWrite(PIN_GATE,HIGH);
  delay(100);
  digitalWrite(PIN_GATE,LOW);
  bot.sendMessage(chat_id, "Ok! Apertei o controle do portão.", "");
  Serial.println("Portao");
}

void loop() {
  connectWiFi();
  if (millis() > telegramLastUpdateTime + TELEGRAM_UPDATE_TIME) {
    int messagesCount = bot.getUpdates(bot.last_message_received + 1);
    while (messagesCount > 0) {
      for (int i = 0; i < messagesCount; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        String text = bot.messages[i].text;
        if (text == "/temperatura") {
          readTemperature(chat_id);
        }else if(text == "/umidade"){
          readHumidity(chat_id);
        }else if(text == "/portao"){
          doGate(chat_id);
        }
      }
      messagesCount = bot.getUpdates(bot.last_message_received + 1);
    }

    telegramLastUpdateTime = millis();
  }
}