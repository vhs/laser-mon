#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <MicrosecondTicker.h>
#include <Config.hpp>
#include <DataSamples.hpp>
#include <FS.h>
#include <ArduinoJson.h>
#include <limits.h>

// Define these in the wifi_creds.hpp file
//const char ssid[] = "YOUR SSID"
//const char wifi_passwd[] = "YOUR WIFI PASSWORD"
#include <wifi_creds.hpp>

#define PWM_PIN 4
#define TTL_PIN 1

//###### Global Data #######//
DataSampler g_dataSampler;
ConfigurationManager g_configMgr;

ESP8266WebServer server(80);
MicrosecondTicker usTicker;

unsigned long g_lastMillisUpdate;

//###### Static functions ########/

void SetRandomSeed()
{
    uint32_t seed;

    // random works best with a seed that can use 31 bits
    // analogRead on a unconnected pin tends toward less than four bits
    seed = analogRead(0);
    delay(1);

    for (int shifts = 3; shifts < 31; shifts += 3)
    {
        seed ^= analogRead(0) << shifts;
        delay(1);
    }

    // Serial.println(seed);
    randomSeed(seed);
}

void DoSample(void*)
{
  g_dataSampler.DoSample(PWM_PIN, TTL_PIN);
  // Come up with another random value and reschedule ourself
  usTicker.attach_us(random(20,800), &DoSample, NULL, true);
}

void handleRoot()
{
  //server.send(200, "text/plain", String(cnt));
}

void handleGetConfig() {
	String str;
	ConfigurationParams cfg = g_configMgr.Get();
	ConfigRepresentationFactory::AsJson(cfg, str);
	server.send(200, "application/json", str);
}

void handlePostConfig()
{
	long secondsPerUpdate = server.arg("secondsPerUpdate").toInt();
	String urlForUpdate = server.arg("urlForUpdate");

	ConfigurationParams cfg = g_configMgr.Get();
	if (secondsPerUpdate)
		cfg.secondsPerUpdate = secondsPerUpdate;
	if (urlForUpdate.length())
		cfg.urlForUpdate = urlForUpdate;
}

void handleGetSamples()
{
	g_dataSampler.Reset();
	SampleData sampleData = g_dataSampler.GetData();

	// jsonify it!
	ArduinoJson::StaticJsonBuffer<256> jsonBuf;
	ArduinoJson::JsonObject& obj = jsonBuf.createObject();

	obj["num_samples"] = sampleData.numMeasurements;

	// Get
	obj["avg_pwr"] = (100 * sampleData.numPwmPinHigh) / (float)sampleData.numMeasurements;

	// TTL is active low
	obj["avg_on"] = (100 * (sampleData.numMeasurements - sampleData.numTtlPinHigh)) / (float)sampleData.numMeasurements;

	unsigned long sampleTime;
	unsigned long currentTime = millis();
	if (currentTime < g_lastMillisUpdate)
	{
		// We rolled over, figure out the amount of roll over
		sampleTime = currentTime + ULONG_MAX - g_lastMillisUpdate;
	}
	else
	{
		sampleTime = currentTime - g_lastMillisUpdate;
	}
	g_lastMillisUpdate = millis();
	obj["sample_time_ms"] = sampleTime;

	String str;
	obj.printTo(str);
	server.send(200, "application/json", str);
}

void SetupWebServer()
{
  server.on("/", handleRoot);
  server.on("/Config", HTTP_GET, handleGetConfig);
  server.on("/Config", HTTP_POST, handlePostConfig);
  server.on("/Samples", HTTP_POST, handleGetSamples);

  server.begin();
  Serial.println("HTTP server started");
}

void DoWifiConnect()
{
  WiFi.begin(ssid, wifi_passwd);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void StartSampling()
{
  usTicker.attach_us(500, &DoSample, NULL, true);
}

int InitializeSpiffs()
{
  bool bFormatted = true;
  bool bSpiffsInit = SPIFFS.begin();
  if (!bSpiffsInit)
  {
    Serial.println("Reformatting spiffs");
    bFormatted = SPIFFS.format();
    bSpiffsInit = SPIFFS.begin();
  }
  if (!bFormatted)
  {
    String msg = "Formatted: ";
    msg = msg + String(bFormatted);
    msg = msg + String(" initialized: ");
    msg = msg + String(bSpiffsInit);
    Serial.println("Could not initialize SPIFFS");
    Serial.println(msg);
  }
  return (bFormatted && bSpiffsInit) ? 0 : -1;
}

void setup(void){
  bool bErrorOnSetup = false;
  int ret = 0;

  SetRandomSeed();
  Serial.begin(115200);

  DoWifiConnect();

  SetupWebServer();

  ret = InitializeSpiffs();
  if (ret < 0)
  {
	  Serial.println("ERROR: initializing SPIFFS");
	  bErrorOnSetup = true;
  }

  ret = g_configMgr.Load();
  if (ret < 0)
  {
	  Serial.println("ERROR: loading configuration");
	  bErrorOnSetup = true;
  }

  if (!bErrorOnSetup)
  {
	  StartSampling();
  }
}

void loop(void){
  server.handleClient();

}


