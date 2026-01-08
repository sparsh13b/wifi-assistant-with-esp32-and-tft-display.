#include "WiFi.h"

const char * ap_ssid = "Wifi_assistant";
const char* ap_password = "244466666";

void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setting up ESP32 Access Point...");
  WiFi.mode(WIFI_AP );

  bool ap_started = WiFi.softAP(ap_ssid, ap_password);

  if(ap_started){
    Serial.println("Access Point started successfully ! ");
    Serial.print("Network Name (SSID): ");
    Serial.println(ap_ssid);
    Serial.print("Password : ");
    Serial.println(ap_password);
    Serial.print("IP_Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Ready for connections !");
  }
  else Serial.println("Failed to start Access Point !");
}

void loop(){

  int connectedDevices = WiFi.softAPgetStationNum();
  static int key = -1;
  static unsigned long int last_count = 0;
  int current_count = millis();

if(millis() - last_count > 7000 ){
  last_count = millis();
  if(connectedDevices != key ){
      key = connectedDevices;
      if(connectedDevices >0){
      Serial.print("Connected Devices  ");
      Serial.println(key);
      }
 else Serial.println("No devices connected ");
  }
}}

