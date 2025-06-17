#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// WiFi credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Telegram Bot info
#define BOT_TOKEN ""
#define CHAT_ID ""

// Pinos do sensor HC-SR04
#define TRIG_PIN 13
#define ECHO_PIN 12

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

// Filas
QueueHandle_t detectQueue;
QueueHandle_t envioQueue;

void taskMovimento(void * parameter) {
  int eventoDetectado = 1;
  long duration;
  float distanceCm;

  for (;;) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);                      
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);                     
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout de 30ms = 5 metros máx.
    distanceCm = duration * 0.0343 / 2;

    if (distanceCm > 0 && distanceCm <= 5) {
      xQueueSend(detectQueue, &eventoDetectado, portMAX_DELAY);
      vTaskDelay(pdMS_TO_TICKS(5000)); // Aguarda 5s antes de detectar de novo
    } else {
      Serial.print("Distância: ");
      Serial.print(distanceCm);
      Serial.println(" cm");

      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}

void taskProcessaEvento(void * parameter) {
  int evento;
  for (;;) {
    if (xQueueReceive(detectQueue, &evento, portMAX_DELAY)) {
      if (evento == 1) {
        xQueueSend(envioQueue, &evento, portMAX_DELAY);
      }
    }
  }
}

void taskTelegram(void * parameter) {
  int evento;
  for (;;) {
    if (xQueueReceive(envioQueue, &evento, portMAX_DELAY)) {
      if (evento == 1) {
        Serial.println("Enviando mensagem Telegram: Movimento detectado");
        bot.sendMessage(CHAT_ID, "Movimento detectado!", "");
      }
    }
  }
}

void setup() {
  Serial.begin(250000);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.print("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
  bot.sendMessage(CHAT_ID, "Bot inicializado!", "");

  detectQueue = xQueueCreate(10, sizeof(int));
  envioQueue = xQueueCreate(10, sizeof(int));
  if (detectQueue == NULL || envioQueue == NULL) {
    Serial.println("Falha ao criar filas!");
    while (1) delay(1000);
  }

  xTaskCreatePinnedToCore(taskMovimento,     "TaskMovimento",     10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskProcessaEvento,"TaskProcessaEvento",10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskTelegram,      "TaskTelegram",      10000, NULL, 1, NULL, 1);
}

void loop() {

}
