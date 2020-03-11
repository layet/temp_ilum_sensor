// Функция инициализации WiFi
bool initWiFi(){  
  Debugln("[initWiFi()] Initializing WiFi");
  // Если не задан SSID в настройках - запускаем точку доступа
  if (Opt.esid == "") {
    Debugln("[initWiFi()]  No SSID in settings. Starting AP.");
    setupAP();
    return false;
  }
  Debug("[initWiFi()]  CONNECTING TO '");
  Debug(Opt.esid);
  Debugln("'");
  // скажем ESP не писать во флеш настройки WiFi при кажом подключении
  WiFi.persistent(false);
  WiFi.disconnect(true);
  // добавляем обработчик событий WiFi
  //WiFi.onEvent(WiFiEvent);
  // включаем режим станции
  delay(100);
  WiFi.mode(WIFI_STA);
  // пробуем подключиться к точке доступа из настроек
  WiFi.begin(Opt.esid.c_str(), Opt.epass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
    Debug(".");
  }
  Debugln("[initWiFi()]  IP: "+WiFi.localIP().toString());
  /*if ( testWifi() == 1 ) { 
    Debugln("[initWiFi()]  IP: "+WiFi.localIP().toString());
    return true;
  }*/
}

// Функция проверки доступности точки доступа
int testWifi(void) {
  int c = 0;  
  Debugln("testWifi()");
  // 30 секунд на подключение
  while ( c < 60 ) {
    if (WiFi.status() == WL_CONNECTED) { 
      Debugln("");
      return(1); 
    } 
    delay(500);
    Debug(".");    
    c++;
  }
  Debugln("");
  return(0);
}

void WiFiEvent(WiFiEvent_t event) {
      switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Debugln("[WiFiEvent()] WiFi connected");
            Debug("[WiFiEvent()] IP address: ");
            Debugln(WiFi.localIP());
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Debugln("[WiFiEvent()] WiFi lost connection");
            break;
    }
}

// Функция запуска точки доступа
void setupAP(void) {
  Debugln("[setupAP()] Launching Access Point");
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP);
  
  // Запускаем точку доступа
  WiFi.softAP(ssid);
  WiFi.begin(ssid);
  IPAddress myIP = WiFi.softAPIP();
  Debug("AP IP address: ");
  Debugln(myIP);
  APMode = 1;
  // Запускаем веб-сервер
  launchWeb();
}

void checkOTAUpdate(void)
{

    Debugln("[setup()] OTA UPDATE ENABLED. CHECKING FOR UPDATES.");
    Debugln(OTAUPDATESERVER);
    Debugln(OTAUPDATEPATH);
    Debugln(OTAUPDATECLASS+String("-")+OTAUPDATEVERSION);
    t_httpUpdate_return ret = ESPhttpUpdate.update(OTAUPDATESERVER, 80, OTAUPDATEPATH, OTAUPDATECLASS+String("-")+OTAUPDATEVERSION);
    switch(ret) {
      case HTTP_UPDATE_FAILED:
        Debugf("[setup()] Update failed. Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        Debugln("");
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Debugln("[setup()] Update no Update.");
        break;
      case HTTP_UPDATE_OK:
        Debugln("[setup()] Update ok.");
        break;
    } 
}
