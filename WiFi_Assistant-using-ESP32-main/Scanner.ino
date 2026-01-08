#include "WiFi.h"

// Function declaration (must be before setup/loop)
String getSecurityType(wifi_auth_mode_t securityType);

void setup() {
  Serial.begin(115200);
  delay(1000);
 
  Serial.println("WiFi Scanner");
 
  // Set WiFi to station mode to scan for networks
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
 
  Serial.println("Starting WiFi scan...");
}

void loop() {
  Serial.println("\n--- Scanning for WiFi Networks ---");
 
  // Start the scan
  int networkCount = WiFi.scanNetworks();
 
  if (networkCount == 0) {
    Serial.println("No networks found");
  } else {
    // Fixed alignment for this line
    Serial.print("Found ");
    Serial.print(networkCount);
    Serial.println(" networks:");
    Serial.println();
   
    // Print header with proper spacing
    Serial.println("No. | SSID                          | RSSI | Security       | Channel | BSSID");
    Serial.println("----+-------------------------------+------+----------------+---------+------------------");
   
    // Scan and display each network
    for (int i = 0; i < networkCount; i++) {
      // Get network information
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      wifi_auth_mode_t security = WiFi.encryptionType(i);
      int32_t channel = WiFi.channel(i);
      String bssid = WiFi.BSSIDstr(i);
      
      // Handle empty SSID
      if (ssid == "") {
        ssid = "[Hidden Network]";
      }
      
      // Print formatted output
      Serial.printf("%2d  | %-29s | %4d | %-14s | %7d | %s\n",
                    i + 1,
                    ssid.c_str(),
                    rssi,
                    getSecurityType(security).c_str(),
                    channel,
                    bssid.c_str());
    }
    
    Serial.println();
    Serial.println("Legend:");
    Serial.println("RSSI: Signal strength (dBm) - Higher = stronger");
    Serial.println("Channel: WiFi channel number");
    Serial.println("BSSID: MAC address of access point");
  }
  
  // Clear scan results
  WiFi.scanDelete();
  
  Serial.println("\nWaiting 30 seconds before next scan...");
  delay(30000);
}

// Function to convert security type to readable string
String getSecurityType(wifi_auth_mode_t securityType) {
  switch (securityType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2 PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2 PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2 Enterprise";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3 PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2/WPA3 PSK";
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI PSK";
    default:
      return "Unknown";
  }
}
