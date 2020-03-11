void webHandleMain(AsyncWebServerRequest *request) {
  Debugln("webHandleMain()");
  String s = "";

  // открываем файл шаблона страницы
  File file = SPIFFS.open("/index.html", "r");
  while (file.available()) { s += file.readStringUntil('\n'); }
  file.close();

  s.replace("%name%", Opt.ename);
  s.replace("%ssid%", Opt.esid);
  s.replace("%pass%", Opt.epass);
  s.replace("%mqtt_server%", Opt.mqttServer);
  s.replace("%mqtt_topic%", Opt.pubTopic);
  s += "\r\n\r\n";
  
  // отправляем HTML клиенту
  request->send(200, "text/html", s);
}

// Функция обработки формы настроек контроллера
void webHandleConfigSave(AsyncWebServerRequest *request){
  Debugln("webHandleConfigSave()");
  String s;
  s = "<p>Settings saved to config file and reset to boot into new settings</p>\r\n\r\n";
  request->send(200, "text/html", s); 
  
  // очистка конфигурации
  clearConfig();

  // Имя контроллера
  String qname; 
  qname = request->getParam("name")->value();
  qname.replace("%2F","/");
  Debugln(qname);
  Debugln("");
  Opt.ename = (char*) qname.c_str();

  // SSID WiFi-сети
  String qsid; 
  qsid = request->getParam("ssid")->value();
  qsid.replace("%2F","/");
  Debugln(qsid);
  Debugln("");
  Opt.esid = (char*) qsid.c_str();

  // Пароль WiFi-сети
  String qpass;
  qpass = request->getParam("pass")->value();
  qpass.replace("%2F","/");
  Debugln(qpass);
  Debugln("");
  Opt.epass = (char*) qpass.c_str();

  // адрес MQTT-сервера
  String qmqttip;
  qmqttip = request->getParam("mqtt_server")->value();
  Debugln(qmqttip);
  Debugln("");
  Opt.mqttServer = (char*) qmqttip.c_str();

  // MQTT топик 
  String qpubTop;
  qpubTop = request->getParam("mqtt_topic")->value();
  qpubTop.replace("%2F","/");
  Debugln(qpubTop);
  Debugln("");
  Opt.pubTopic = (char*) qpubTop.c_str();

  Debug("Settings written ");
  saveConfig()? Debugln("sucessfully.") : Debugln("not succesfully!");
  Debugln("Restarting!"); 
  delay(1000);
  
  // перезагрузка контроллера
  ESP.reset();
}

void launchWeb() {    
  // Настраиваем роутер веб-сервера
    Debugln("[launchWeb()] Launching web server");
    server.on("/", HTTP_GET, webHandleMain);
    server.serveStatic("/matmin.css", SPIFFS, "/matmin.css");
    server.on("/cfgsave", HTTP_GET, webHandleConfigSave);
    server.onNotFound(onRequest);
    server.begin();
}

// Обработчик необработанных запросов сервера
void onRequest(AsyncWebServerRequest *request){
  // Выдадим ошибку HTTP 404
  request->send(404);
}
