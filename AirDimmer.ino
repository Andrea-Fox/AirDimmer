#include "network_information.h"


#include <Wire.h>
#include "SparkFun_VL53L1X.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

const char* devicename = "test"; // sets MQTT topics and hostname for ArduinoOTA


//*******************************************************************************************************************
char mqtt_serial_publish_distance_ch_cache[50];
char mqtt_serial_receiver_ch_cache[50];
char mqtt_serial_publish_measurements_cache[64];


int mqtt_distance = sprintf(mqtt_serial_publish_distance_ch_cache,"%s%s%s","AirDimmer/", devicename,"/distance");
const PROGMEM char* mqtt_serial_publish_distance_ch = mqtt_serial_publish_distance_ch_cache;
int mqtt_receiver = sprintf(mqtt_serial_receiver_ch_cache,"%s%s%s","AirDimmer/", devicename,"/receiver");
const PROGMEM char* mqtt_serial_receiver_ch = mqtt_serial_receiver_ch_cache;

int mqtt_publish_measurements = sprintf(mqtt_serial_publish_measurements_cache,"%s%s%s","AirDimmer/", devicename,"/publish_measurements");
const PROGMEM char* mqtt_serial_publish_measurements_ch = mqtt_serial_publish_measurements_cache;


#define EEPROM_SIZE 8

WiFiClient espClient;
PubSubClient client(espClient);

//Optional interrupt and shutdown pins
#define SHUTDOWN_PIN 2    
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor(Wire);//, SHUTDOWN_PIN, INTERRUPT_PIN);



void setup_wifi() 
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
    // WiFi.begin(ssid, password);
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("connected");
      client.subscribe(mqtt_serial_receiver_ch);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}




void setup(void)
{
  Wire.begin();
  // initialize the EEPROM memory
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.begin(9600);

  if (distanceSensor.init() == false)
    Serial.println("Sensor online!");
  distanceSensor.setIntermeasurementPeriod(100);
  distanceSensor.setDistanceModeLong();

  Serial.setTimeout(500);// Set time out for setup_wifi();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(1000);
  reconnect();  
  
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.setHostname(devicename);
  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



void loop(void)
{
  ArduinoOTA.handle();


  

}
