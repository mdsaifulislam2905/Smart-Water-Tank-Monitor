// ============================================================
//  Smart Water Monitoring System — NodeMCU ESP8266 Firmware
// ============================================================

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>          
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ─────────────────────────────────────────────
#define WIFI_SSID        "UIU-STUDENT"       
#define WIFI_PASSWORD    "12345678"   

#define FIREBASE_HOST    "smartwatertank-c24f5-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH    "asDXlSqCCvAd9h7pA1rpkj15IkNKU7A5neTyFOcs"
// ─────────────────────────────────────────────

#define TRIG_PIN      D5   
#define ECHO_PIN      D6   
#define TEMP_PIN      D4   
#define TURBIDITY_PIN A0   
#define RELAY_PIN     D7   

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1   
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

FirebaseData fbData;          
FirebaseData fbStream;        
FirebaseAuth  fbAuth;
FirebaseConfig fbConfig;

const float LEVEL_EMPTY_CM = 25.0; 
const float LEVEL_FULL_CM  =  4.0;

const int TURBIDITY_RAW_MIN = 0;
const int TURBIDITY_RAW_MAX = 640;

bool manualPumpOverride  = false;  
bool manualPumpState     = false;  
bool oledEnabled         = true;
bool ultrasonicEnabled   = true;
bool tempEnabled         = true;
bool turbidityEnabled    = true;

unsigned long lastSensorRead  = 0;
unsigned long lastFirebasePush = 0;
const unsigned long SENSOR_INTERVAL  = 1000;  
const unsigned long FIREBASE_INTERVAL = 1000; 

float currentDistanceCm = 0.0; 
float waterLevelPercent = 0.0;
float temperatureC      = 0.0;
float turbidityPercent  = 0.0;
bool  pumpIsOn          = false;

// ============================================================
float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) return -1.0;                 
  return (duration * 0.0343) / 2.0;               
}

float distanceToLevel(float distanceCm) {
  float level = map(distanceCm * 100, LEVEL_FULL_CM * 100, LEVEL_EMPTY_CM * 100, 10000, 0);
  return constrain(level / 100.0, 0.0, 100.0);
}

float readTurbidity() {
  int raw = analogRead(TURBIDITY_PIN);
  raw = constrain(raw, TURBIDITY_RAW_MIN, TURBIDITY_RAW_MAX);
  float pct = map(raw, TURBIDITY_RAW_MIN, TURBIDITY_RAW_MAX, 0, 100);
  return pct;
}

void setPump(bool on) {
  pumpIsOn = on;
  if (on) {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);  
  } else {
    pinMode(RELAY_PIN, INPUT);     
  }
}

void updateOLED() {
  if (!oledEnabled) {
    display.clearDisplay();
    display.display();
    return;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(10, 0);
  display.print(F("Smart Water Monitor"));
  display.drawLine(0, 9, SCREEN_WIDTH - 1, 9, SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print(F("Level: "));
  display.print(currentDistanceCm, 1);
  display.print(F("cm ["));
  display.print(waterLevelPercent, 0);
  display.print(F("%]"));

  display.setCursor(0, 28);
  display.print(F("Temp : "));
  display.print(temperatureC, 1);
  display.print(F(" C"));

  display.setCursor(0, 41);
  display.print(F("Water: "));
  if (turbidityPercent < 50.0) {
    display.print(F("Dirty"));
  } else {
    display.print(F("Clean"));
  }
  display.print(F(" ("));
  display.print(turbidityPercent, 0);
  display.print(F("%)"));

  display.setCursor(0, 54);
  display.print(F("Pump : "));
  if (millis() < 10000) {
     display.print(F("WAIT..."));
  } else {
     display.print(pumpIsOn ? F("ON") : F("OFF"));
  }

  display.display();
}

void pushSensorData() {
  FirebaseJson json;
  json.set("waterLevel", waterLevelPercent);
  json.set("waterDistance", currentDistanceCm); 
  json.set("temperature", temperatureC);
  json.set("turbidity",   turbidityPercent);
  json.set("waterStatus", turbidityPercent < 50.0 ? "Dirty" : "Clean"); 
  json.set("pumpState",   pumpIsOn);
  json.set("timestamp",   (int)millis());
  Firebase.updateNode(fbData, "/sensors", json);
}

void readControlFlags() {
  if (Firebase.getJSON(fbData, "/controls")) {
    FirebaseJsonData result;
    fbData.jsonObject().get(result, "manualPumpOverride");
    if (result.success) manualPumpOverride = result.boolValue;

    fbData.jsonObject().get(result, "manualPumpState");
    if (result.success) manualPumpState = result.boolValue;

    fbData.jsonObject().get(result, "oledEnabled");
    if (result.success) oledEnabled = result.boolValue;

    fbData.jsonObject().get(result, "ultrasonicEnabled");
    if (result.success) ultrasonicEnabled = result.boolValue;

    fbData.jsonObject().get(result, "tempEnabled");
    if (result.success) tempEnabled = result.boolValue;

    fbData.jsonObject().get(result, "turbidityEnabled");
    if (result.success) turbidityEnabled = result.boolValue;
  }
}

void evaluatePumpLogic() {
  if (millis() < 10000) {
    setPump(false); 
    return;         
  }

  if (manualPumpOverride) {
    setPump(manualPumpState);
  } else {
    bool autoPump = (turbidityPercent < 50.0);
    setPump(autoPump);
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN,  OUTPUT);
  pinMode(ECHO_PIN,  INPUT);
  
  pinMode(RELAY_PIN, INPUT_PULLUP); 
  setPump(false); 

  Wire.begin(D2, D1);  
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 24);
    display.println(F("Connecting WiFi..."));
    display.display();
  }

  tempSensor.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

  fbStream.setResponseSize(4096);
  Firebase.beginStream(fbStream, "/controls");

  FirebaseJson defaults;
  defaults.set("manualPumpOverride", false);
  defaults.set("manualPumpState",    false);
  defaults.set("oledEnabled",        true);
  defaults.set("ultrasonicEnabled",  true);
  defaults.set("tempEnabled",        true);
  defaults.set("turbidityEnabled",   true);
  Firebase.updateNode(fbData, "/controls", defaults);
}

void loop() {
  unsigned long now = millis();

  if (Firebase.readStream(fbStream) && fbStream.streamAvailable()) {
    readControlFlags();  
    evaluatePumpLogic(); 
    updateOLED();        
  }

  if (!fbStream.httpConnected()) {
    Firebase.beginStream(fbStream, "/controls");
  }

  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;

    if (ultrasonicEnabled) {
      float dist = readUltrasonic();
      if (dist > 0) {
        currentDistanceCm = dist; 
        waterLevelPercent = distanceToLevel(dist);
      }
    }

    if (tempEnabled) {
      tempSensor.requestTemperatures();
      float tempReading = tempSensor.getTempCByIndex(0);
      
      // MAGIC FILTER: Only accept realistic values (Ignore -127 or completely wrong data)
      if (tempReading != DEVICE_DISCONNECTED_C && tempReading > -50.0) {
        temperatureC = tempReading;
      } else {
        Serial.println(F("[Temp] Error: Sensor connection lost! Retaining last valid value."));
      }
    }

    if (turbidityEnabled) {
      turbidityPercent = readTurbidity();
    }

    evaluatePumpLogic();
    updateOLED();
  }

  if (now - lastFirebasePush >= FIREBASE_INTERVAL) {
    lastFirebasePush = now;
    if (Firebase.ready()) {
      pushSensorData();    
      readControlFlags();  
    }
  }
}