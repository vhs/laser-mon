#ifndef __CONFIG__H__
#define __CONFIG__H__

#include <stdint.h>
#include <FS.h>
#include <ArduinoJson.h>

#define CONFIG_FILE_NAME "/configuration"

#define CONFIG_VERSION 1

#define CONFIG_JSON_MAX_LENGTH 255

struct ConfigurationParams
{
  ConfigurationParams()
    : version(0)
    , secondsPerUpdate(0)
    , urlForUpdate("Blarg")
  {}

  uint32_t version;
  uint32_t secondsPerUpdate;
  String urlForUpdate;
};

class ConfigRepresentationFactory
{
public:
	static void AsJson(ConfigurationParams const& config, String &str)
	{
		ArduinoJson::StaticJsonBuffer<255> jsonBuf;
		ArduinoJson::JsonObject& obj = jsonBuf.createObject();
		obj["secondsPerUpdate"] = static_cast<int>(config.secondsPerUpdate);
		obj["urlForUpdate"] = config.urlForUpdate;
		obj.printTo(str);
	}
};

class ConfigurationManager
{
public:
  ConfigurationManager()
    : m_conf()
  {}

  int Load()
  {
    File configFile = SPIFFS.open(CONFIG_FILE_NAME, "r");
    if (!configFile)
    {
      Serial.println("Could not open config file");
      return -1;
    }
    configFile.readBytes((uint8_t*) &m_conf.version, 4);

    Serial.println(String("Read version ") + String(m_conf.version));

    //char buf[CONFIG_JSON_MAX_LENGTH+1];
    String cfg = configFile.readString();
    Serial.print("Read from cfgFile ");
    Serial.println(cfg);


    configFile.close();

    ArduinoJson::StaticJsonBuffer<256> jsonBuffer;
    ArduinoJson::JsonObject& jsonObject = jsonBuffer.parseObject(cfg, 2);


    if (jsonObject == ArduinoJson::JsonObject::invalid())
    {
    	Serial.println("Error parsing json");
    	return -1;
    }

    ConfigurationParams cfgToLoad;
    if (jsonObject.containsKey("secondsPerUpdate") && jsonObject.containsKey("urlForUpdate"))
    {
    cfgToLoad.secondsPerUpdate = jsonObject["secondsPerUpdate"];
    cfgToLoad.urlForUpdate = jsonObject["urlForUpdate"].asString();
    }
    else
    {
    	Serial.println("Loaded json object missing \"secondsPerUpdate\" and/or \"urlForUpdate\"");
    	return -1;
    }

    Serial.println("Read configuration:");
    Serial.print(" urlForUpdate: ");
    Serial.println(cfgToLoad.urlForUpdate);
    Serial.print(" secondsPerUpdate: ");
    Serial.println(cfgToLoad.secondsPerUpdate);

    m_conf = cfgToLoad;
    return 0;
  }

  int Save()
  {
    File configFile = SPIFFS.open(CONFIG_FILE_NAME, "w");
    if (!configFile)
    {
      Serial.println("Could not open config file for writing...");
      return -1;
    }
    uint32_t version = CONFIG_VERSION;
    configFile.write((const uint8_t*) &version, 4);

    ArduinoJson::StaticJsonBuffer<255> jsonBuf;
    ArduinoJson::JsonObject& root = jsonBuf.createObject();
    root["secondsPerUpdate"] = static_cast<int>(m_conf.secondsPerUpdate);
    root["urlForUpdate"] = m_conf.urlForUpdate;

    Serial.print("Writing config to flash: ");
    root.printTo(Serial);
    Serial.println("");

    root.printTo(configFile);

    configFile.close();
    return 0;

  }

  int Set(ConfigurationParams conf)
  {
    if (conf.urlForUpdate.length() > 100)
    {
      // Lets just not save this...
      return -1;
    }
    m_conf = conf;
    return 0;

  }

  ConfigurationParams Get() const
  {
    return m_conf;
  }

private:
  ConfigurationParams m_conf;
};

#endif
