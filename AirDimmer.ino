#include "network_information.h"
#include "webpage.h"

#include <Wire.h>
#include "SparkFun_VL53L1X.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>        // or ESPAsyncWebServer for async
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

const char* devicename = "test"; // sets MQTT topics and hostname for ArduinoOTA


//*******************************************************************************************************************
char mqtt_serial_publish_distance_ch_cache[50];
char mqtt_serial_receiver_ch_cache[50];
char mqtt_serial_publish_measurements_cache[64];
char mqtt_serial_publish_brightness_cache[64];

int mqtt_distance = sprintf(mqtt_serial_publish_distance_ch_cache,"%s%s%s","AirDimmer/", devicename,"/distance");
const PROGMEM char* mqtt_serial_publish_distance_ch = mqtt_serial_publish_distance_ch_cache;
int mqtt_receiver = sprintf(mqtt_serial_receiver_ch_cache,"%s%s%s","AirDimmer/", devicename,"/receiver");
const PROGMEM char* mqtt_serial_receiver_ch = mqtt_serial_receiver_ch_cache;



int mqtt_publish_measurements = sprintf(mqtt_serial_publish_measurements_cache,"%s%s%s","AirDimmer/", devicename,"/publish_measurements");
const PROGMEM char* mqtt_serial_publish_measurements_ch = mqtt_serial_publish_measurements_cache;


int mqtt_publish_brightness_change = sprintf(mqtt_serial_publish_brightness_cache,"%s%s%s","AirDimmer/", devicename,"/brightness_change");
const PROGMEM char* mqtt_serial_publish_brighness_change_ch = mqtt_serial_publish_brightness_cache;


#define EEPROM_SIZE 8

WiFiClient espClient;
PubSubClient client(espClient);


char messageArray[50];
//Optional interrupt and shutdown pins
#define SHUTDOWN_PIN 2    
#define INTERRUPT_PIN 3

SFEVL53L1X distanceSensor(Wire);//, SHUTDOWN_PIN, INTERRUPT_PIN);


float lower_threshold = 0.9;    // the lower bound of the line that will actually be considered is lower_threshold * floor_distance
float upper_threshold = 0.1;    // the upper bound of the line that will actually be considered is upper_threshold * floor_distance

const int N_measurements = 5;
int last_measurement_index = 0;
int surface_distance;
float running_sum = 0;
float running_average = 0;
int last_N_measurements[N_measurements];
float initial_height = 0;
float last_height = 0;
float current_height = 0;
float difference = 0;


float lower_bound_threshold = 0.1;
float upper_bound_threshold = 0.9;

bool handDetected = false;
bool previousHandDetected = false;
int delayBetweenMeasurements = 50;

bool update_raw_measurements = false;
// ***************************************************************
// Webserver code

WebServer server(80);
bool armed = false;
bool debug = false;


void handleRoot() {
  server.send(200, "text/html", webpageHTML); // paste the HTML string here
}

void handleDataJSON() {
  String json = "{";
  json += "\"raw\":" + String(0) + ",";
  json += "\"filtered\":" + String(0) + ",";
  json += "\"brightness\":" + String(0) + ",";
  json += "\"armed\":" + String(0) + ",";
  json += "\"debug\":" + String(0);
  json += "}";
  server.send(250, "application/json", json);
}

void handleToggleArmed() {
  armed = !armed;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggleDebug() {
  debug = !debug;
  server.sendHeader("Location", "/");
  server.send(303);
}



// ****************************************************************

void setup_wifi() {
  WiFi.mode(WIFI_STA); // Set Wi-Fi to station mode
  delay(100);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to Wi-Fi");
  
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) { // 20s timeout
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    Serial.println("Check SSID, password, and signal strength.");
  }
}


float threshold_calibration(){
  
  client.publish(mqtt_serial_publish_distance_ch, "Computation of new threshold");
  Serial.println("Computation of new threshold");
  
  float sum_distances = 0;
  uint16_t distance;
  int n_attempts = 20;
  for (int i=0; i<n_attempts; i++){
      // increase sum of values in Zone 0
      // distanceSensor.setROI(ROI_height, ROI_width), center[0]);  // first value: height of the zone, second value: width of the zone
      delay(50);
      distanceSensor.setTimingBudgetInMs(50);
      distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
      distance = distanceSensor.getDistance()/10; //Get the result of the measurement from the sensor
      distanceSensor.stopRanging();      
      sum_distances = sum_distances + distance;
  }
  // average distance from the reflection surface 
  float mean_distance = sum_distances/n_attempts;
  
  return mean_distance;
}


void callback(char* topic, byte *payload, unsigned int length) {
    String newTopic = topic;
    payload[length] = '\0';
    String newPayload = String((char *)payload);    
    if (newTopic == mqtt_serial_receiver_ch) 
    {
      if (newPayload == "threshold_calibration")
      {
        threshold_calibration();
      }
      if (newPayload == "update_raw_measurements")
      {
        update_raw_measurements = true;
      }
      if (newPayload == "stop_update_raw_measurements")
      {
        update_raw_measurements = false;
      }
    }
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

void handleDataRequest() {
    String json = "{";
    // Sensor Data
    json += "\"raw\":" + String(last_N_measurements[last_measurement_index]) + ",";
    // Calculate average safely
    float avg = (N_measurements > 0) ? (running_sum / N_measurements) : 0;
    json += "\"filtered\":" + String(avg) + ",";
    
    json += "\"hand\":" + String(handDetected ? "true" : "false") + ",";
    
    // Output
    // Ensure currentBrightness is defined globally
    json += "\"brightness\":" + String(0) + ",";
    
    // Connectivity
    json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    // If you don't use MQTT, set this to false or remove it
    json += "\"mqtt_connected\":" + String("false"); 
    
    json += "}";
    
    // KEY CHANGE: Use server.send instead of request->send
    server.send(200, "application/json", json);
}


void setup(void){
  Wire.begin();
  // initialize the EEPROM memory
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.begin(115200);

  if (distanceSensor.init() == false)
    Serial.println("Sensor online!");
  distanceSensor.setIntermeasurementPeriod(100);
  distanceSensor.setDistanceModeLong();

  Serial.setTimeout(500);// Set time out for setup_wifi();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(1000);
  // reconnect();  
  /*
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
*/
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


  server.on("/", handleRoot);
  server.on("/data.json", handleDataJSON);
  server.on("/toggle/armed", handleToggleArmed);
  server.on("/toggle/debug", handleToggleDebug);

  // Aggiungi questo nel tuo setup del server (dove hai server.on)
  

  server.on("/", []() {
    server.send(250, "text/html", webpageHTML);
  });

  // 2. Serve the data for the JavaScript fetch
  server.on("/readData", handleDataRequest);

  server.begin();
  Serial.println("Web server started");



  // Here we should have the initialization function
  surface_distance = threshold_calibration();
  Serial.print("Surface distance: ");
  Serial.println(surface_distance);


  // initialization of the last_N_measurement vector
  for(int i; i<N_measurements; i++){
    last_N_measurements[i] = 0;
  }

  delay(500);
}


bool detect_hand(int last_measurements[N_measurements], bool hand_detected){
// bool detect_hand(int last_measurement, bool hand_detected)
  int hand_in_sight = 0;
  for (int i=0; i<N_measurements; i++){
    if ((last_measurements[i] < surface_distance * upper_bound_threshold) && (last_measurements[i] > surface_distance * lower_bound_threshold)){
  //if ((last_measurement < surface_distance * upper_bound_threshold) && (last_measurement > surface_distance * lower_bound_threshold)){
      hand_in_sight ++;
    }
  }
  bool temp_handDetected = false;
  if (hand_detected == true){
    if (hand_in_sight >= N_measurements -1){
    //if (hand_in_sight > 0){
      temp_handDetected = true;
    }
  }
  else{
    if (hand_in_sight == N_measurements){
    //if (hand_in_sight == 1){
      temp_handDetected = true;
    }
  }
  return temp_handDetected;
}


void loop(void)
{
  // ArduinoOTA.handle();
  server.handleClient();

   

  // ArduinoOTA.handle();
  uint16_t distance;
  client.loop();
  if (!client.connected()) 
  {
    reconnect();
  }
  
  delay(50);
  distanceSensor.setTimingBudgetInMs(50);
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.stopRanging();

  //Serial.println(distance);
  distance = distance / 10;

  // CIRCULAR BUFFER
  
  running_sum = running_sum - last_N_measurements[last_measurement_index];
  last_N_measurements[last_measurement_index] = distance;
  running_sum += distance;
  last_measurement_index ++;
  last_measurement_index = last_measurement_index % N_measurements;
 

  // First we check if the hand is detected
  // handDetected = detect_hand(last_N_measurements, handDetected);
  handDetected = detect_hand(last_N_measurements, handDetected);

  current_height = running_sum/(N_measurements);
  // If the hand has been detected, but before it was not there, 
  if (handDetected == true && previousHandDetected == false){
    Serial.println("Hand detected");
    // distanceSensor.setTimingBudgetInMs(50);
    // distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
    // distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
    // distanceSensor.stopRanging();
    initial_height = distance;
    last_height = distance;
    running_sum = distance * N_measurements;
    for(int i; i<N_measurements; i++){
      last_N_measurements[i] = distance;
    }
    last_measurement_index = 0;
    difference = 0;
  }
  if (handDetected == true && previousHandDetected == true){
    difference = last_height - current_height;

  }

  if (previousHandDetected == true && last_measurement_index == 0 && handDetected == true){
    Serial.print(handDetected);
    Serial.print("\tSurface = ");
    Serial.print(surface_distance);
    Serial.print("\tInitial height = ");
    Serial.print(initial_height);
    Serial.print("\tLast height = ");
    Serial.print(last_height);
    Serial.print("\tCurrent height = ");
    Serial.print(current_height);
    Serial.print("\tDifferenza = ");
    Serial.println(difference);
    
    //String stringaDifference = String(difference); //+ stringaZona + String(zona) + "\t" + String(PathTrack[0]) + String(PathTrack[1]) + String(PathTrack[2]);
    // stringaDifference.toCharArray(messageArray, stringaDifference.length() +1);
    if (difference  >=3 || difference <= -3){
      difference = min(max(difference, -20.0f), 20.0f);
      client.publish(mqtt_serial_publish_brighness_change_ch, String(difference/2).c_str());
    }
    // client.publish(mqtt_serial_publish_brighness_change_ch, "Ciao");
    last_height = current_height;

  }

  if (update_raw_measurements == true){
    client.publish(mqtt_serial_publish_distance_ch, String(distance).c_str());
  }
  


  previousHandDetected = handDetected;
}
