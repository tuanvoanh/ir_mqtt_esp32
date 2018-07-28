//#include "ESP32_IR_Remote.h"
#include <IRremote.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define RECV_PIN 5
#define SEND_PIN 18


#define ssid      "CRETA-VNPT"            // Set you WiFi SSID
#define password  "yoursolution"              // Set you WiFi password

#define mqtt_server  "cretatech.com"
#define port 1883

#define SENDTOPIC "IR/key"
#define COMMANDTOPIC "IR/command"
#define SERVICETOPIC "IR/service"

void setup_wifi();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect();
void mqtt_callback();
//void storage_ir();
void receiveEnable();
void receiveIR();
void getMess();
void sendCode( int codeType, unsigned long codeValue, int codeLen);
