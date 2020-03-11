// Функция удаления конфигурации контроллера
bool clearConfig(){
    Debugln("clearConfig");
    configToClear = 0;
    return SPIFFS.remove("/config.cfg");  
}

// Функция загрузки конфигурации контроллера
bool loadConfig(){
  Debugln("loadConfig()");
  File configFile = SPIFFS.open("/config.cfg", "r");
  if (!configFile) {
    Debugln("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Debugln("Config file size is too large");
    return false;
  }

  String ename = configFile.readStringUntil('\n');
  String esid = configFile.readStringUntil('\n');
  String epass = configFile.readStringUntil('\n');
  String emqttServer = configFile.readStringUntil('\n');
  String emqttTopic = configFile.readStringUntil('\n');

  Opt.ename = ename;
  Opt.esid = esid;
  Opt.epass = epass;
  Opt.mqttServer = emqttServer;
  Opt.pubTopic = emqttTopic;

  Debug("ename: ");
  Debugln(Opt.ename);
  Debug("esid: ");
  Debugln(Opt.esid);
  Debug("epass: ");
  Debugln(Opt.epass);
  Debug("mqttServer: ");
  Debugln(Opt.mqttServer);
  Debug("pubTopic: ");
  Debugln(Opt.pubTopic);

  return true;
}

bool saveConfig() {
  Debugln("saveConfig()");
  
  String s = Opt.ename;
  s += "\n";
  s += Opt.esid;
  s += "\n";
  s += Opt.epass;
  s += "\n";
  s += Opt.mqttServer;
  s += "\n";
  s += Opt.pubTopic;
  s += "\n";

  File configFile = SPIFFS.open("/config.cfg", "w");
  if (!configFile) {
    Debugln("Failed to open config file for writing");
    return false;
  }
  
  configFile.print(s);
  configFile.close();
  return true;
}
