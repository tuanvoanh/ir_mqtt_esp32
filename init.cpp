#include "init.h"
WiFiClient wifiClient;
//ESP32_IRrecv irrecv;
PubSubClient client(wifiClient);

//unsigned int IRdata[1000]; //holding IR code in ms
//unsigned int IRdataSend[1000]; //holding IR code in ms

decode_results results;
//
IRrecv irrecv(RECV_PIN);
IRsend irsend(SEND_PIN);
//json Object
DynamicJsonBuffer jsonBuffer(200);
JsonObject& root = jsonBuffer.createObject();
JsonObject& rootData = jsonBuffer.createObject();
// Storage for the recorded code
int codeType = -1; // The type of code
unsigned long codeValue; // The code value if not raw
unsigned int rawCodes[RAWBUF]; // The durations if raw
int codeLen; // The length of the code
extern int IRmode = 0; // 0: nothing
                // 1: learn
                // 2: send
int toggle = 0; // The RC5/6 toggle state
void setup_wifi(){

  delay(10);
  // We start by connecting to a WiFi network
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
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String IRcommand = "";
  for (int i = 0; i < length; i++) {
    IRcommand = IRcommand + (char)payload[i];
  }
  DynamicJsonBuffer jsonBuffer(200);
  JsonObject& message = jsonBuffer.parseObject(payload);

  // Test if parsing succeeds.
  if (!message.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if (message["func"] == "learn" ){
    IRmode = 1;
    Serial.print("learn Mode"); 
    receiveEnable();
  }
  if (message["func"] == "play"){
    sendCode(message["data"]["type"],message["data"]["value"],message["data"]["length"]);
  }
}

void mqtt_connect(){
  client.setServer(mqtt_server, port);
  
}
void mqtt_callback(){
  client.setCallback(callback);
  reconnect();
  client.subscribe(COMMANDTOPIC);
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMQTT(String topic, String message) {
  Serial.print("sending to MQTT topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.print(message);
  if (!client.connected()) {
    reconnect();
  }
  //client.publish(topic.c_str(), "hello");
  client.publish(topic.c_str(), message.c_str());
}

//void storage_ir(){
//    irrecv.ESP32_IRrecvPIN(RECV_PIN,0);//channel 0 so it can use the full memory of the channels
//    irrecv.initReceive();
//    Serial.println("Init complete");
//    Serial.println("Send an IR to Copy");
//    int codeLen=0;
//    codeLen=irrecv.readIR(IRdata,sizeof(IRdata));
//    if (codeLen > 50) { //ignore any short codes
////      digitalWrite(16,0);
//        for (int i = 0;i < (codeLen+1); i++){
//            IRdataSend[i] = IRdata[i];
//        }
//        irrecv.stopIR(); //uninstall the RMT channel so it can be reused for Receiving IR or send IR
//        DynamicJsonBuffer jsonBuffer(5000);
//        JsonObject& root = jsonBuffer.createObject();
//        DynamicJsonBuffer jsonBuffer1(200);
//        JsonArray& array = jsonBuffer1.createArray();
//        JsonObject& root_data = jsonBuffer.createObject();
//        array.copyFrom(IRdataSend);
//        root["function"] = "learn";
//        root_data["data"] = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012";
//        root_data["len"] = codeLen;
//        root["data"] = root_data;
//        //root.prettyPrintTo(Serial);
//        String IRcommand = "";
//        root.printTo(IRcommand);
//        
//        publishMQTT(SENDTOPIC, IRcommand);
//    }
//}
void receiveEnable(){
  irrecv.enableIRIn();
  
}

void storeCode(decode_results *results) {
  codeType = results->decode_type;
  //int count = results->rawlen;
  if (codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    return;
  }
  else {
    if (codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(codeType, DEC);
      Serial.println("");
      return;
    }
    Serial.println(results->value, HEX);
    codeValue = results->value;
    codeLen = results->bits;
    root["func"] = "learn";
    rootData["type"] = codeType;
    rootData["value"] = results->value;
    rootData["length"] = results->bits;
    root["data"]= rootData;
    String IRcommand= "";
    root.printTo(IRcommand);
    publishMQTT(SENDTOPIC, IRcommand);
  }
}
void receiveIR(){
   if (irrecv.decode(&results)) {
      storeCode(&results);
      irrecv.resume();
      irrecv.disableIRIn();
      IRmode = 0;
    }       
}
void getMess(){
  client.loop();
}
void sendCode( int codeType, unsigned long codeValue, int codeLen) {
  Serial.print("send: ");
  if (codeType == NEC) {
    irsend.sendNEC(codeValue, codeLen);
    Serial.print("Sent NEC ");
    Serial.println(codeValue, HEX);
    Serial.println(codeLen);
  }
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == JVC) {
    irsend.sendJVC(codeValue, codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == RC5 || codeType == RC6) {
    toggle = 1 - toggle;
    // Put the toggle bit into the code to send
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    }
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  }
  else if (codeType == UNKNOWN /* i.e. raw */) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}


