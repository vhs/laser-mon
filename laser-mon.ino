#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Ticker.h>
#include "SFE_BMP180.h"

// Define these in the config.h file
//#define WIFI_SSID "yourwifi"
//#define WIFI_PASSWORD "yourpassword"
//#define INFLUX_HOSTNAME "data.example.com"
//#define INFLUX_PORT 8086
//#define INFLUX_PATH "/write?db=<database>&u=<user>&p=<pass>"
//#define WEBSERVER_USERNAME "something"
//#define WEBSERVER_PASSWORD "something"
#include "config.h"

#define DEVICE_NAME "vhs-laser-mon"

#define BMP180_A_SDA 4
#define BMP180_A_SCL 2
#define BMP180_B_SDA 12
#define BMP180_B_SCL 13
#define ONE_WIRE_PIN 14
#define PSU_PWM 16
#define PSU_TTL 5

#define SENSOR_FREQ 5000
#define UPLOAD_FREQ 5000

struct measurement {
  double temperature;
  double pressure;
  bool valid;
};




extern "C" {
#include "user_interface.h"
}

os_timer_t pwm_timer;
unsigned long ttl_accum = 0;
unsigned long pwm_accum = 0;
unsigned long pwm_count = 0;
void timerCallback(void *pArg) {
  pwm_accum += digitalRead(PSU_PWM);
  ttl_accum += !digitalRead(PSU_TTL);
  pwm_count++;
}






#define N_SENSORS 1
byte sensorAddr[][8] = {
  {0x28, 0xFF, 0xD4, 0xA9, 0xA1, 0x15, 0x04, 0x1C}, // (chiller-bath)
};
char * sensorNames[] = {
  "chiller_bath",
};

#include "libdcc/webserver.h"
#include "libdcc/onewire.h"
#include "libdcc/influx.h"

unsigned long lastSensorIteration;
unsigned long lastUploadIteration;


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  server.on("/restart", handleRestart);
  server.on("/status", handleStatus);
  server.on("/sensors", handleSensors);
  server.onNotFound(handleNotFound);
  server.begin();

  // Fire the timer every millisecond.
  // This isn't technically accurate in determining the duty cycle of the 20khz PWM signal,
  // but seems to work well in practice.
  os_timer_setfn(&pwm_timer, timerCallback, NULL);
  os_timer_arm(&pwm_timer, 1, true);
  
  // Initialize zeroth iteration
  takeAllMeasurementsAsync();
  lastSensorIteration = millis();
  // Offset upload events from sensor events
  lastUploadIteration = millis() + SENSOR_FREQ / 2;
}
WiFiClient client;

void loop() {
  server.handleClient();

  // Copy HTTP client response to Serial
  while (client.connected() && client.available()) {
    Serial.print(client.readStringUntil('\r'));
  }

  // If we are NOT ready to do a sensor iteraction, return early
  if (millis() < lastSensorIteration + SENSOR_FREQ) {
    return;
  }
  lastSensorIteration = millis();

  Serial.println(String("PWM: ") + pwm_accum + "/" + pwm_count);
  Serial.println(String("TTL: ") + ttl_accum + "/" + pwm_count);
  String sensorBody = String(DEVICE_NAME) + " uptime=" + String(millis()) + "i";
  sensorBody += String(",psu_pwm=") + String((float)pwm_accum/pwm_count, 3);
  sensorBody += String(",psu_ttl_avg=") + String((float)ttl_accum/pwm_count, 3);
  sensorBody += String(",psu_ttl=") + !digitalRead(PSU_TTL) + "i";
  ttl_accum = 0;
  pwm_accum = 0;
  pwm_count = 0;
  

  float temp[N_SENSORS];
  for (int i=0; i<N_SENSORS; i++) {
    Serial.print("Temperature sensor ");
    Serial.print(i);
    Serial.print(": ");
    if (readTemperature(sensorAddr[i], &temp[i])) {
      Serial.print(temp[i]);
      Serial.println();
      sensorBody += String(",") + sensorNames[i] + "=" + String(temp[i], 3);
    }
  }

  measurement m1 = getMeasurement(BMP180_A_SDA, BMP180_A_SCL);
  if (m1.valid) {
    sensorBody += String(",m1_t=") + String(m1.temperature, 3);
    sensorBody += String(",m1_p=") + String(m1.pressure, 3);
  }
  measurement m2 = getMeasurement(BMP180_B_SDA, BMP180_B_SCL);
  if (m2.valid) {
    sensorBody += String(",m2_t=") + String(m2.temperature, 3);
    sensorBody += String(",m2_p=") + String(m2.pressure, 3);
  }


  Serial.println(sensorBody);

  // Instruct temperature sensors to take measurements for next iteration
  takeAllMeasurementsAsync();

  // If we are ready to do an upload iteration, do that now
  if (millis() > lastUploadIteration + UPLOAD_FREQ) {
    Serial.println("Wifi connected to " + WiFi.SSID() + " IP:" + WiFi.localIP().toString());
    client.connect(INFLUX_HOSTNAME, INFLUX_PORT);
    postRequestAsync(sensorBody, client);
    lastUploadIteration = millis();
  }
}

measurement getMeasurement(unsigned char sda, unsigned char scl) {
  char status;
  measurement m;
  m.valid = false;

  SFE_BMP180 pressure;
  pressure.begin(sda, scl);

  status = pressure.startTemperature();
  if (status == 0) return m;
  delay(status);

  status = pressure.getTemperature(m.temperature);
  if (status == 0) return m;

  status = pressure.startPressure(3);
  if (status == 0) return m;
  delay(status);

  status = pressure.getPressure(m.pressure, m.temperature);
  if (status == 0) return m;

  m.valid = true;
  return m;
}


