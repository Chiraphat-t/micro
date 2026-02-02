#include <WiFi.h>
#include <MQTT.h>
#include <AccelStepper.h>

// --- การตั้งค่าขาใช้งาน ---
#define BUTTON_PIN    18
#define LED_PIN       26
#define LDR_PIN       34
#define DC_IN1        22
#define DC_IN2        21
#define DC_EN         23

// Stepper ULN2003 (IN1, IN3, IN2, IN4)
AccelStepper stepper(AccelStepper::FULL4WIRE, 19, 17, 18, 16); 

// --- ตัวแปรสำหรับสถานะ ---
int dcState = 0;       // 0=หยุด, 1=ตามเข็ม, 2=ทวนเข็ม
int lastDir = 1;       // จำทิศทางล่าสุด
unsigned long lastMsg = 0;

WiFiClient net;
MQTTClient client;

void connect() {
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  while (!client.connect("Mix_Project_ESP32")) { delay(500); }
  client.subscribe("groupAi4/motor/command");
  client.subscribe("groupAi4/stepper/command");
}

void messageReceived(String &topic, String &payload) {
  payload.toLowerCase();

  // --- ส่วนควบคุม DC Motor ด้วย If-Else ---
  if (topic == "groupAi4/motor/command") {
    if (payload == "on") {
      if (dcState == 0) {
        dcState = lastDir; // ถ้าหยุดอยู่ ให้หมุนตามทิศเดิม
      } else if (dcState == 1) {
        dcState = 2;       // ถ้าหมุนขวาอยู่ ให้สลับซ้าย
      } else {
        dcState = 1;       // ถ้าหมุนซ้ายอยู่ ให้สลับขวา
      }
    } 
    else if (payload == "off") {
      dcState = 0;
    } 
    else if (payload == "cw") {
      dcState = 1;
    } 
    else if (payload == "ccw") {
      dcState = 2;
    }
  }

  // --- ส่วนควบคุม Stepper Motor ---
  if (topic == "groupAi4/stepper/command") {
    if (payload == "open") {
      stepper.moveTo(0);
    } else if (payload == "close") {
      stepper.moveTo(7500);
    }
  }
}

void setup() {
  pinMode(DC_IN1, OUTPUT);
  pinMode(DC_IN2, OUTPUT);
  pinMode(DC_EN, OUTPUT);
  digitalWrite(DC_EN, HIGH); // ให้ Driver ทำงานตลอด
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  WiFi.begin("@JumboPlusIoT", "07450748");
  client.begin("test.mosquitto.org", net);
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  client.loop();
  stepper.run();
  if (!client.connected()) connect();

  // --- เช็คปุ่มกดที่บอร์ดด้วย If-Else ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(250); // กันปุ่มสั่นเบื้องต้น
    if (dcState == 0) { dcState = 1; }
    else if (dcState == 1) { dcState = 2; }
    else { dcState = 0; }
  }

  // --- การสั่งงาน Hardware ตาม dcState ---
  if (dcState == 0) {
    digitalWrite(DC_IN1, LOW);
    digitalWrite(DC_IN2, LOW);
    digitalWrite(LED_PIN, LOW);
  } 
  else if (dcState == 1) {
    digitalWrite(DC_IN1, HIGH);
    digitalWrite(DC_IN2, LOW);
    digitalWrite(LED_PIN, HIGH);
    lastDir = 1;
  } 
  else if (dcState == 2) {
    digitalWrite(DC_IN1, LOW);
    digitalWrite(DC_IN2, HIGH);
    digitalWrite(LED_PIN, HIGH);
    lastDir = 2;
  }

  // --- ส่งค่า LDR กลับไป MQTT ---
  if (millis() - lastMsg > 2000) {
    lastMsg = millis();
    client.publish("groupAi4/status/ldr", String(analogRead(LDR_PIN)));
  }
}
