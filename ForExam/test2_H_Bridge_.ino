#define ENABLE 23
#define INPUT1 32
#define INPUT2 33
#define BUTTON 27

#include <LCD_I2C.h>
#include <WiFi.h>
#include <MQTT.h>

// LCD
LCD_I2C lcd(0x27, 16, 2); // Default address of most PCF8574 modules

// WiFi and MQTT Configuration
const char ssid[] = "NUTCHA";
const char pass[] = "1122334455";

const char mqtt_broker[] = "test.mosquitto.org";
const char mqtt_topic[] = "group226/command";  // Command topic
const char mqtt_topic2[] = "group226/status";  // Status topic
const char mqtt_client_id[] = "clientId-uniqueID"; // Replace with unique client ID
int MQTT_PORT = 1883;

// State Variables
int count = 0;
int result = 0;
int prevResult = -1; // Previous state to track changes

WiFiClient net;
MQTTClient client;

// Function to connect to WiFi and MQTT
void connect() {
  Serial.print("Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi Connected!");

  Serial.print("Connecting to MQTT...");
  while (!client.connect(mqtt_client_id)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nMQTT Connected!");

  client.subscribe(mqtt_topic); // Subscribe to command topic
}

// Callback for MQTT messages
void messageReceived(String &topic, String &payload) {
  Serial.println("Message received:");
  Serial.println("  Topic: " + topic);
  Serial.println("  Payload: " + payload);

  // Handle incoming commands
  if (payload == "0") {
    result = 0;
  } else if (payload == "1") {
    result = 1;
  } else if (payload == "2") {
    result = 2;
  } else {
    Serial.println("Unknown command received");
  }

  // Update outputs and LCD based on command
  if (result != prevResult) {
    prevResult = result; // Update previous state

    if (result == 0) {
      stop();
      lcd.clear();
      lcd.print("Stop");
    } else if (result == 1) {
      clockWise();
      lcd.clear();
      lcd.print("CounterClockwise");
    } else if (result == 2) {
      reClockWise();
      lcd.clear();
      lcd.print("Clockwise");
    }

    // Publish the updated state to MQTT
    publishState();
  }
}

// Function to publish state to MQTT
void publishState() {
  String state;
  if (result == 0) {
    state = "Stop";
  } else if (result == 1) {
    state = "CounterClockwise";
  } else if (result == 2) {
    state = "Clockwise";
  }

  if (client.publish(mqtt_topic2, state.c_str())) {
    Serial.println("Published: " + state);
  } else {
    Serial.println("Failed to publish state");
  }
}

void setup() {
  // Serial and LCD Initialization
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();

  // Pin Setup
  pinMode(ENABLE, OUTPUT); // Enable Pin
  pinMode(INPUT1, OUTPUT); // Direction Pin 1
  pinMode(INPUT2, OUTPUT); // Direction Pin 2
  pinMode(BUTTON, INPUT_PULLUP);

  // WiFi and MQTT Setup
  WiFi.begin(ssid, pass);
  client.begin(mqtt_broker, MQTT_PORT, net);
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  // Maintain MQTT connection
  if (!client.connected()) {
    connect();
  }
  client.loop();

  // Read button and debounce
  if (digitalRead(BUTTON) == LOW) {
    delay(500); // Debounce delay
    count++;
    result = count % 3; // Cycle between 0, 1, 2

    // Update outputs and LCD only if result has changed
    if (result != prevResult) {
      prevResult = result; // Update previous state

      if (result == 0) {
        stop();
        lcd.clear();
        lcd.print("Stop");
      } else if (result == 1) {
        reClockWise();
        lcd.clear();
        lcd.print("CounterClockwise");
      } else if (result == 2) {
        clockWise();
        lcd.clear();
        lcd.print("Clockwise");
      }

      // Publish state to MQTT
      publishState();
    }
  }
}

// Rotate counter-clockwise
void reClockWise() {
  digitalWrite(ENABLE, HIGH);
  digitalWrite(INPUT1, LOW);
  digitalWrite(INPUT2, HIGH);
}

// Rotate clockwise
void clockWise() {
  digitalWrite(ENABLE, HIGH);
  digitalWrite(INPUT1, HIGH);
  digitalWrite(INPUT2, LOW);
}

// Stop the motor
void stop() {
  digitalWrite(ENABLE, LOW);  // Disable motor driver (optional, depends on your driver)
  digitalWrite(INPUT1, LOW);
  digitalWrite(INPUT2, LOW);
}
