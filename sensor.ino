#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>


//Definindo Variaveis Principais

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define BOT_TOKEN ""
#define CHAT_ID ""

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);


const int pirPin = 32;
int state = 0;

WiFiServer server(80);

void setup() {
  pinMode(pirPin, INPUT);
  Serial.begin(9600);


  //Comecando conexao
  Serial.println();
  Serial.println("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
      delay(100);
      Serial.print(".");

  }

  Serial.println("");
  Serial.println("Wifi Conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
  
  configTime(0,0, "pool.ntp.org");
  time_t now = time(nullptr);
  while(now < 24 * 3600){
    WiFiClientSecure
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  bot.sendMessage(CHAT_ID, "Bot Start", "");
  

}

void loop() {
  // put your main code here, to run repeatedly:
  state = digitalRead(pirPin);
  if (state == HIGH){
    Serial.println("Moveu!");
    bot.sendMessage(CHAT_ID, "Moveu!");
    state = digitalRead(pirPin);
  }else{
    Serial.println("Procurando...");
  }
  delay(200);
}
