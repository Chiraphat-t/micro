#include <WiFi.h>
#include <MQTT.h>

// --- Pins ---
//#define MOTOR_ENABLE 23
#define MOTOR_INPUT1 22
//#define MOTOR_INPUT2 21
//#define LED_SINGLE   4   
#define BUTTON_PIN   18 
#define LDR 34

// --- Topics ---
const char mqtt_client_id[] = "AI_Group4";
const char mqtt_topic[]    = "groupAi4/command";
const char status_motor[]  = "groupAi4/status/motor";
//const char status_led[]    = "groupAi4/status/led"; // Topic สำหรับสถานะไฟบน Dashboard
const char status_ldr[]    = "groupAi4/status/ldr";
const char status_mode[]    = "groupAi4/status/mode";

WiFiClient net;
MQTTClient client;
bool isAutoMode = true;
bool motorStatus = false;
int ldrValue = 0;
          
volatile bool buttonPressed = false;
unsigned long lastInterruptTime = 0;
volatile int currentState = 0;   

void IRAM_ATTR handleButtonInterrupt() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > 250) {
    currentState++;
    if (currentState > 2) currentState = 0;
    buttonPressed = true;
    lastInterruptTime = interruptTime;
  }
}

// ... (ส่วน connect, messageReceived, setup, loop เหมือนเดิม) ...

void connect() {
  Serial.print("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  while (!client.connect(mqtt_client_id)) { delay(500); }
  client.subscribe(mqtt_topic);
  publishStatus();
}

void messageReceived(String &topic, String &payload) {
  if (payload == "on") motorStatus = true;
  else if (payload == "off" ) motorStatus = false;
  else if (payload == "auto") isAutoMode=true;
  else if (payload == "manual") isAutoMode=false;
  publishStatus();
  
}
void publishStatus(){
  client.publish(status_motor, motorStatus ? "ON" : "OFF");
  client.publish(status_mode, isAutoMode ? "Auto" : "Manual");
  client.publish(status_ldr, String(ldrValue));
}

void setup() {
  Serial.begin(115200);
  //pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR_INPUT1, OUTPUT);
  //pinMode(MOTOR_INPUT2, OUTPUT);
  //pinMode(LED_SINGLE, OUTPUT);
  //digitalWrite(MOTOR_ENABLE, HIGH);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

  WiFi.begin("@JumboPlusIoT", "07450748");
  client.begin("test.mosquitto.org", net);
  client.onMessage(messageReceived);
  connect();
  
}

void loop() {
  client.loop();
  if (!client.connected()) connect();
  if (buttonPressed) {
    if(isAutoMode){
      isAutoMode=false;
    }
    else {isAutoMode=true;}
    buttonPressed = false;
  }
  ldrValue = analogRead(LDR);
  if(isAutoMode){
    if(ldrValue<400){
      motorStatus = true;
    }
    else motorStatus = false;
  }
  digitalWrite(MOTOR_INPUT1, motorStatus ? HIGH : LOW);

  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    publishStatus();
    lastUpdate = millis();
  }
}
