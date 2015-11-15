#include <stdlib.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#define BUTTON          2
const char* ssid = "AMnetgear";
const char* password = "malibu2515";

int sensorState = HIGH;
int lastSensorState;
char* topic = "water_detect";
char* server = "192.168.1.20";


WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Wire.pins(0,2);
  Wire.begin();
  // set button pin as an input
  pinMode(BUTTON, INPUT_PULLUP);

  
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
  mqttConnect();
  
}

void mqttConnect()
{
  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  clientName += "esp8266-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  Serial.print("Connecting to ");
  Serial.print(server);
  Serial.print(" as ");
  Serial.println(clientName);
  
  
  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT broker");
    Serial.print("Topic is: ");
    Serial.println(topic);
    
    if (client.publish(topic, "Status OK")) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
}
bool mqttCheck(String clientName)
{

if (!client.connected())
return client.connect((char*) clientName.c_str());
else
return true;
}

void loop() {
  
  
  String payload;
  // grab the current state of the button
  sensorState = digitalRead(BUTTON);
  if (sensorState != lastSensorState){
    if (sensorState == LOW) {
      String payload = "WATER DETECTED!!";
      Serial.println(payload); 
    
      if (client.connected()){
        Serial.print("Sending payload: ");
        Serial.println(payload);
    
        if (client.publish(topic, (char*) payload.c_str())) {
          Serial.println("Publish ok");
        }
        else {
          Serial.println("Publish failed");
          mqttConnect();
        }
      
      }
    }
   else {
      payload = "Status OK";
      client.connected();
      Serial.print("Sending payload: ");
      Serial.println(payload);
    
      if (client.publish(topic, (char*) payload.c_str())) {
        Serial.println("Publish ok");
      }
      else {
        Serial.println("Publish failed");
        mqttConnect();
      }
  
  }
  lastSensorState = sensorState;
  
  
  }}

 
 


