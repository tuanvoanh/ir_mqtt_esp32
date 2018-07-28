#include "init.h"
extern int IRmode; // 0: nothing
                   // 1: learn
                   // 2: send
void setup() {
  pinMode(4,OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  mqtt_connect();
  mqtt_callback();
  reconnect();
  //storage_ir();
  //receiveEnable();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (IRmode == 1){
    receiveIR();  
  }
  
  getMess();
}
