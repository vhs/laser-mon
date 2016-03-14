#define MQTT_MAX_PACKET_SIZE 256

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <PubSubClient.h>

#include <MicrosecondTicker.h>

#include <DataSamples.hpp>
#include <ArduinoJson.h>
#include <limits.h>


// Define these in the wifi_creds.hpp file
//const char ssid[] = "YOUR SSID"
//const char wifi_passwd[] = "YOUR WIFI PASSWORD"
#include <wifi_creds.hpp>

#define PWM_PIN D0
#define TTL_PIN D1

//###### Global Data #######//
DataSampler g_dataSampler;

MicrosecondTicker usTicker;

WiFiClient client;

static const char* k_mqttbroker = "172.16.0.161";
PubSubClient g_pubSubClient(client);


unsigned long g_lastMillisUpdate;

#define MQTT_TOPIC "cook/laser/power"

//###### Static functions ########/

void DoSample(void*)
{
  g_dataSampler.DoSample(PWM_PIN, TTL_PIN);
  // Come up with another random value and reschedule ourself
  usTicker.attach_us(800, &DoSample, NULL, true);
}

void reconnect() {
  // Loop until we're reconnected
  while (!g_pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (g_pubSubClient.connect("Laser Power Monitor")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //g_pubSubClient.publish("outTopic", "hello world");
      // ... and resubscribe
      //g_pubSubClient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(g_pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

	//obj["numTtlPinHigh"] = sampleData.numTtlPinHigh;
	//obj["numPwmPinHigh"] = sampleData.numPwmPinHigh;

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

	reconnect();

	String str;
	obj.printTo(str);
	//Serial.print("Publishing: ");
	Serial.println(str);

	bool ret = g_pubSubClient.publish(MQTT_TOPIC, str.c_str());
	if (!ret)
	{
		Serial.println("Failed to publish");
	}
	//server.send(200, "application/json", str);
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
  Serial.println("StartSampling()");
  usTicker.attach_us(500, &DoSample, NULL, true);
}

void setup(void){

  Serial.begin(115200);

  DoWifiConnect();

  g_pubSubClient.setServer(k_mqttbroker,1883);

  reconnect();

  StartSampling();

}

void loop(void)
{
	delay(1000);
	handleGetSamples();
	//if (!g_pubSubClient.connected())
	//{
	//  reconnect();
	//}

}


