#define DEBUG
// включаем или отключаем режим отладки

#ifdef DEBUG
  #define Debug(x)    Serial.print(x)
  #define Debugln(x)  Serial.println(x)
  #define Debugf(...) Serial.printf(__VA_ARGS__)
  #define Debugflush  Serial.flush
#else
  #define Debug(x)    {}
  #define Debugln(x)  {}
  #define Debugf(...) {}
  #define Debugflush  {}
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include <Ticker.h>
#include "wemos.h"

#define OTAUPDATECLASS "TEMP_ILUM_SENSOR" // OTA: Имя класса устройства
#define OTAUPDATEVERSION "0.1.1" // OTA: Версия прошивки устройства
#define OTAUPDATESERVER "ota.i-alice.ru" // OTA: Сервер обновления
#define OTAUPDATEPATH "/update/index.php" // OTA: Путь обновления на сервере
#define ONE_WIRE_BUS D5

// Структура настроек контроллера
typedef struct
{
  String ename = "";
  String esid = "";
  String epass = "";
  String pubTopic = "";
  String mqttServer = "";
} options;

// Переменные для чтения настроек из SPIFFS
options Opt;
// SSID в режиме точки доступа
const char *ssid = "tempIlumSensor";

//##### Глобальные переменные ##### 
int reset_button_counter = 0;
String topic_to_send = "";
String msg_to_send = "";
char buf[40]; // буфер для данных MQTT

//##### Флаги ##### нужны, чтобы не выполнять длительные операции вне цикла loop
int configToClear=0; // флаг стирания конфигурации
int APMode=0;        // режим работы (клиент/точка доступа)
int toPub=0; // флаг публикации MQTT

//##### Объекты #####
Ticker reset_timer;
Ticker work_timer;
AsyncWebServer server(80);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress thermometer;

void setup() {
  #ifdef DEBUG
    Serial.begin(9600, SERIAL_8N1);
  #endif
  pinMode(D1, INPUT_PULLUP);
  Debugln();
  if (!SPIFFS.begin()) {
    Debugln("[setup()] FAILED TO MOUNT FILE SYSTEM!");
  }
  // загружаем настройки
  if (!loadConfig()) { Debugln("[setup()] loadConfig() FAILED!"); }
  initWiFi();
  // если соединение прошло успешно обновление контроллера (OTA)
  if (APMode==0) {
    checkOTAUpdate();
    connectMQTT();
  }
  sensors.begin();
  if (!sensors.getAddress(thermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(thermometer, 9);
  
  reset_timer.attach(0.1, reset_handle);
  work_timer.attach(300, work_handle);
}

void reset_handle()
{
  int ed1 = digitalRead(D1);
  
  if (ed1 == 1) reset_button_counter = 0;
  if (ed1 == 0) reset_button_counter++;

  if (reset_button_counter > 20) configToClear = 1;
}

void work_handle()
{
  int val = analogRead(A0);
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(thermometer);
  pubMQTT("/ilum", (char*)String(val).c_str());
  pubMQTT("/temp", (char*)String(tempC).c_str());
  /*topic_to_send = "/ilum";
  msg_to_send = String(val);
  toPub = 1;*/
}

void loop() {
  // Если установлен флаг сброса настроек
  if(configToClear==1){
    // сбрасываем настройки и перезагружаемся
    clearConfig();
    delay(1000);
    ESP.reset();
  }
  // Обработка MQTT-сообщений
  if (APMode==0) mqtt_handler();
}
