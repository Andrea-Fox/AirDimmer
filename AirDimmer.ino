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

char deviceSuffix[32] = "setup"; // Default suffix
char fullHostname[64];

char mqtt_serial_publish_distance_ch_cache[64];
char mqtt_serial_receiver_ch_cache[64];
char mqtt_serial_publish_measurements_cache[64];
char mqtt_serial_publish_brightness_cache[64];

const char* mqtt_serial_publish_distance_ch = mqtt_serial_publish_distance_ch_cache;
const char* mqtt_serial_receiver_ch = mqtt_serial_receiver_ch_cache;
const char* mqtt_serial_publish_measurements_ch = mqtt_serial_publish_measurements_cache;
const char* mqtt_serial_publish_brighness_change_ch = mqtt_serial_publish_brightness_cache;

void updateFullHostname() {
  if (strlen(deviceSuffix) == 0 || strcmp(deviceSuffix, "none") == 0) {
    snprintf(fullHostname, sizeof(fullHostname), "airdimmer");
  } else {
    snprintf(fullHostname, sizeof(fullHostname), "airdimmer-%s", deviceSuffix);
  }
  
  // MQTT topics use ONLY the suffix as requested
  snprintf(mqtt_serial_publish_distance_ch_cache, 64, "AirDimmer/%s/distance", deviceSuffix);
  snprintf(mqtt_serial_receiver_ch_cache, 64, "AirDimmer/%s/receiver", deviceSuffix);
  snprintf(mqtt_serial_publish_measurements_cache, 64, "AirDimmer/%s/publish_measurements", deviceSuffix);
  snprintf(mqtt_serial_publish_brightness_cache, 64, "AirDimmer/%s/brightness_change", deviceSuffix);
}


#define EEPROM_SIZE 128  // Increased to store hostname suffix

// EEPROM Memory Map
#define EEPROM_MAGIC_ADDR 0        // 4 bytes - magic number
#define EEPROM_SURFACE_DIST_ADDR 4 // 4 bytes - float
#define EEPROM_UPPER_THRESH_ADDR 8 // 4 bytes - float
#define EEPROM_LOWER_THRESH_ADDR 12 // 4 bytes - float
#define EEPROM_ARMED_ADDR 16       // 1 byte - bool
#define EEPROM_UPDATE_RAW_ADDR 17  // 1 byte - bool
#define EEPROM_SENSITIVITY_ADDR 18 // 1 byte - int (1, 2, 3)
#define EEPROM_DEVICE_SUFFIX_ADDR 20 // 32 bytes - char array suffix

#define EEPROM_MAGIC_NUMBER 0xAD14  // Updated magic number for sensitivity feature

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
float sensitivity = 0;


float lower_bound_threshold = 0.1;
float upper_bound_threshold = 0.9;

bool handDetected = false;
bool previousHandDetected = false;
int delayBetweenMeasurements = 50;

bool update_raw_measurements = false;
int currentBrightness = 0;  // Stores the current brightness from MQTT
float lastChangeValue = 0;  // Stores the last change sent to HA
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
  saveConfigToEEPROM();  // Save to EEPROM when changed
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggleDebug() {
  debug = !debug;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleToggleRawMeasurements() {
  update_raw_measurements = !update_raw_measurements;
  saveConfigToEEPROM();  // Save to EEPROM when changed
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCalibrateThreshold() {
  surface_distance = threshold_calibration();
  Serial.print("New surface distance: ");
  Serial.println(surface_distance);
  saveConfigToEEPROM();  // Save to EEPROM after calibration
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleUpdateThresholds() {
  if (server.hasArg("upper") && server.hasArg("lower")) {
    float newUpper = server.arg("upper").toFloat();
    float newLower = server.arg("lower").toFloat();
    
    // Validate values are in [0, 1] range
    if (newUpper >= 0 && newUpper <= 1 && newLower >= 0 && newLower <= 1) {
      upper_bound_threshold = newUpper;
      lower_bound_threshold = newLower;
      
      Serial.print("Thresholds updated - Upper: ");
      Serial.print(upper_bound_threshold);
      Serial.print(", Lower: ");
      Serial.println(lower_bound_threshold);
      
      saveConfigToEEPROM();  // Save to EEPROM when changed
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Values must be between 0 and 1");
    }
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

// ****************************************************************
// EEPROM Functions

void saveConfigToEEPROM() {
  EEPROM.writeUInt(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_NUMBER);
  EEPROM.writeFloat(EEPROM_SURFACE_DIST_ADDR, surface_distance);
  EEPROM.writeFloat(EEPROM_UPPER_THRESH_ADDR, upper_bound_threshold);
  EEPROM.writeFloat(EEPROM_LOWER_THRESH_ADDR, lower_bound_threshold);
  EEPROM.writeBool(EEPROM_ARMED_ADDR, armed);
  EEPROM.writeBool(EEPROM_UPDATE_RAW_ADDR, update_raw_measurements);
  EEPROM.writeByte(EEPROM_SENSITIVITY_ADDR, (uint8_t)sensitivity);
  EEPROM.writeString(EEPROM_DEVICE_SUFFIX_ADDR, deviceSuffix);
  EEPROM.commit();
  Serial.println("Configuration saved to EEPROM");
}

void loadConfigFromEEPROM() {
  uint32_t magic = EEPROM.readUInt(EEPROM_MAGIC_ADDR);
  
  if (magic == EEPROM_MAGIC_NUMBER) {
    surface_distance = EEPROM.readFloat(EEPROM_SURFACE_DIST_ADDR);
    upper_bound_threshold = EEPROM.readFloat(EEPROM_UPPER_THRESH_ADDR);
    lower_bound_threshold = EEPROM.readFloat(EEPROM_LOWER_THRESH_ADDR);
    armed = EEPROM.readBool(EEPROM_ARMED_ADDR);
    update_raw_measurements = EEPROM.readBool(EEPROM_UPDATE_RAW_ADDR);
    sensitivity = (int)EEPROM.readByte(EEPROM_SENSITIVITY_ADDR);
    if (sensitivity < 1 || sensitivity > 3) sensitivity = 1;
    String storedSuffix = EEPROM.readString(EEPROM_DEVICE_SUFFIX_ADDR);
    if (storedSuffix.length() > 0) {
      strncpy(deviceSuffix, storedSuffix.c_str(), 31);
    }
    
    updateFullHostname();
    Serial.println("Configuration loaded from EEPROM:");
    Serial.print("  Hostname: "); Serial.println(fullHostname);
  } else {
    Serial.println("No valid configuration found, using defaults");
    surface_distance = 100.0;
    upper_bound_threshold = 0.9;
    lower_bound_threshold = 0.1;
    armed = true;
    update_raw_measurements = false;
    sensitivity = 1; // 1 (high), 2 (medium), 3 (low)
    strncpy(deviceSuffix, "setup", 31);
    updateFullHostname();
    saveConfigToEEPROM();
  }
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
      else if (newPayload == "update_raw_measurements")
      {
        update_raw_measurements = true;
      }
      else if (newPayload == "stop_update_raw_measurements")
      {
        update_raw_measurements = false;
      }
      else
      {
        // Try to parse as brightness value (0-255 from Home Assistant)
        int brightnessValue = newPayload.toInt();
        // Validate it's a reasonable brightness value (0-255 range)
        if (brightnessValue >= 0 && brightnessValue <= 255)
        {
          // Normalize from 0-255 to 0-100
          currentBrightness = (int)((brightnessValue / 255.0) * 100.0);
          Serial.print("Brightness received: ");
          Serial.print(brightnessValue);
          Serial.print(" -> Normalized to: ");
          Serial.println(currentBrightness);
        }
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

// Helper to safely format floats for JSON
String safeJsonFloat(float val, int precision = 2) {
  if (isnan(val) || isinf(val)) return "0.0";
  return String(val, precision);
}

void handleDataRequest() {
    String json = "{";
    // Sensor Data
    json += "\"raw\":" + String(last_N_measurements[last_measurement_index]) + ",";
    // Calculate average safely
    float avg = (N_measurements > 0) ? (running_sum / N_measurements) : 0;
    json += "\"filtered\":" + safeJsonFloat(avg, 1) + ",";
    
    json += "\"hand\":" + String(handDetected ? "true" : "false") + ",";
    
    // Output
    json += "\"brightness\":" + String(currentBrightness) + ",";
    json += "\"lastChange\":" + safeJsonFloat(lastChangeValue, 1) + ",";
    
    // Settings
    json += "\"armed\":" + String(armed ? "true" : "false") + ",";
    json += "\"update_raw_measurements\":" + String(update_raw_measurements ? "true" : "false") + ",";
    json += "\"upper_threshold\":" + safeJsonFloat(upper_bound_threshold, 3) + ",";
    json += "\"lower_threshold\":" + safeJsonFloat(lower_bound_threshold, 3) + ",";
    json += "\"surface_distance\":" + safeJsonFloat(surface_distance, 1) + ",";
    json += "\"sensitivity\":" + String(sensitivity) + ",";
    json += "\"device_suffix\":\"" + String(deviceSuffix) + "\",";
    json += "\"full_hostname\":\"" + String(fullHostname) + ".local\",";
    
    // Connectivity - Use 1/0 for absolute robustness in JS
    json += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? 1 : 0) + ",";
    json += "\"mqtt_connected\":" + String(client.connected() ? 1 : 0); 
    
    json += "}";
    
    server.send(200, "application/json", json);
}

void handleUpdateHostname() {
  if (server.hasArg("suffix")) {
    String suffix = server.arg("suffix");
    suffix.trim();
    if (suffix.length() > 0 && suffix.length() < 31) {
      strncpy(deviceSuffix, suffix.c_str(), 31);
      saveConfigToEEPROM();
      server.send(200, "text/plain", "OK. Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      server.send(400, "text/plain", "Invalid suffix length");
    }
  } else {
    server.send(400, "text/plain", "Missing suffix parameter");
  }
}


void handleUpdateSensitivity() {
  if (server.hasArg("val")) {
    int val = server.arg("val").toInt();
    if (val >= 1 && val <= 3) {
      sensitivity = val;
      saveConfigToEEPROM();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid sensitivity");
    }
  } else {
    server.send(400, "text/plain", "Missing val");
  }
}

void setup(void){
  Wire.begin();
  // initialize the EEPROM memory
  EEPROM.begin(EEPROM_SIZE);
  
  Serial.begin(115200);

  // Load configuration from EEPROM immediately after EEPROM.begin
  loadConfigFromEEPROM();
  
  // If surface_distance is invalid or this is first boot, calibrate
  if (surface_distance <= 0 || surface_distance > 400) {
    Serial.println("Invalid surface distance, performing calibration...");
    surface_distance = threshold_calibration();
    saveConfigToEEPROM();
  }
  
  Serial.print("Surface distance: ");
  Serial.println(surface_distance);


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
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

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
  
  ArduinoOTA.setHostname(fullHostname);
  ArduinoOTA.setPassword("admin"); // Security: Add OTA password
  ArduinoOTA.begin();

  if (MDNS.begin(fullHostname)) {
    Serial.println("MDNS responder started");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("arduino", "tcp", 3232); // Explicitly advertise OTA port
  }

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);



  server.on("/", handleRoot);
  server.on("/toggle/armed", handleToggleArmed);
  server.on("/toggle/debug", handleToggleDebug);
  server.on("/toggle/rawMeasurements", handleToggleRawMeasurements);
  server.on("/calibrate/threshold", handleCalibrateThreshold);
  server.on("/update/thresholds", handleUpdateThresholds);
  server.on("/update/sensitivity", handleUpdateSensitivity);
  server.on("/update/hostname", handleUpdateHostname);
  server.on("/readData", handleDataRequest);

  server.begin();
  Serial.println("Web server started");

  saveConfigToEEPROM();
  


  // initialization of the last_N_measurement vector
  for(int i=0; i<N_measurements; i++){  
    last_N_measurements[i] = 0;
  }

  delay(500);
}


bool detect_hand(int last_measurements[], bool hand_detected){
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
  ArduinoOTA.handle();
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
    for(int i=0; i<N_measurements; i++){
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
      if (armed) {  // Only publish if armed is true
        difference = min(max(difference, -20.0f), 20.0f);
        lastChangeValue = difference / (float)sensitivity;  // Sensitivity acts as a divider
        client.publish(mqtt_serial_publish_brighness_change_ch, String(lastChangeValue).c_str());
      }
    }
    // client.publish(mqtt_serial_publish_brighness_change_ch, "Ciao");
    last_height = current_height;

  }
  
  // Reset last change when hand is not detected
  if (handDetected == false) {
    lastChangeValue = 0;
  }

  if (update_raw_measurements == true){
    client.publish(mqtt_serial_publish_distance_ch, String(distance).c_str());
  }
  


  previousHandDetected = handDetected;
}
