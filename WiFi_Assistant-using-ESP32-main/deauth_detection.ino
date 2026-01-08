
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>


uint32_t total_deauth_packets = 0;
uint32_t attack_count = 0;
unsigned long last_stats_print = 0;

uint8_t current_channel = 1;
unsigned long last_channel_switch = 0;
const uint32_t CHANNEL_SWITCH_INTERVAL = 10000; // 10 seconds per channel
bool full_scan_completed = false;
uint8_t scan_cycles = 0;

uint32_t deauth_per_channel[14] = {0}; // Index 0 unused, channels 1-13
uint32_t attacks_per_channel[14] = {0};

typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6]; // Destination MAC
    uint8_t addr2[6]; // Source MAC  
    uint8_t addr3[6]; // BSSID
    unsigned sequence_ctrl:16;
} __attribute__((packed)) wifi_ieee80211_mac_hdr_t;

typedef struct {
    wifi_ieee80211_mac_hdr_t hdr;
    uint8_t payload[0];
} __attribute__((packed)) wifi_ieee80211_packet_t;

unsigned long deauth_times[10];
uint8_t time_index = 0;

String macToString(const uint8_t* mac) {
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(mac_str);
}

void switchChannel() {
    current_channel++;
    if (current_channel > 13) {
        current_channel = 1;
        full_scan_completed = true;
        scan_cycles++;
        Serial.printf("\n*** FULL SCAN CYCLE %d COMPLETED ***\n", scan_cycles);
    }
    
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    Serial.printf("\n>>> Switched to channel %d <<<\n", current_channel);
    last_channel_switch = millis();
    
    for (int i = 0; i < 10; i++) {
        deauth_times[i] = 0;
    }
    time_index = 0;
}

void packetSniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
   
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_ieee80211_packet_t* packet = (wifi_ieee80211_packet_t*)pkt->payload;
   
    uint16_t frame_control = packet->hdr.frame_ctrl;
    uint8_t frame_type = (frame_control & 0x0C) >> 2;
    uint8_t frame_subtype = (frame_control & 0xF0) >> 4;
   
    if (frame_type == 0 && frame_subtype == 12) {
        total_deauth_packets++;
        deauth_per_channel[current_channel]++;
       
        Serial.printf("[CH%d DEAUTH] From: %s To: %s BSSID: %s\n",
                     current_channel,
                     macToString(packet->hdr.addr2).c_str(),
                     macToString(packet->hdr.addr1).c_str(),
                     macToString(packet->hdr.addr3).c_str());
       
        unsigned long now = millis();
        deauth_times[time_index] = now;
        time_index = (time_index + 1) % 10;
       
        uint8_t recent_count = 0;
        for (int i = 0; i < 10; i++) {
            if (now - deauth_times[i] <= 2000) recent_count++;
        }
       
        if (recent_count > 3) {
            attack_count++;
            attacks_per_channel[current_channel]++;
            Serial.printf("*** DEAUTH ATTACK DETECTED ON CHANNEL %d! *** (Attack #%lu)\n", 
                         current_channel, attack_count);
            Serial.printf("*** %d deauth packets in last 2 seconds ***\n", recent_count);
        }
    }
}

void printChannelStats() {
    Serial.println("\n=== CHANNEL ACTIVITY REPORT ===");
    bool activity_found = false;
    
    for (int ch = 1; ch < 14; ch++) {
        if (deauth_per_channel[ch] > 0) {
            activity_found = true;
            Serial.printf("Channel %2d: %lu deauth packets, %lu attacks detected\n", 
                         ch, deauth_per_channel[ch], attacks_per_channel[ch]);
        }
    }
    
    if (!activity_found) {
        Serial.println("No deauth activity detected on any channel");
    }
    
    Serial.println("\n=== SUMMARY ===");
    Serial.printf("Total deauth packets: %lu\n", total_deauth_packets);
    Serial.printf("Total attacks detected: %lu\n", attack_count);
    Serial.printf("Runtime: %lu seconds\n", millis()/1000);
    
    // Find most active channel
    uint8_t most_active_channel = 0;
    uint32_t max_deauths = 0;
    for (int ch = 1; ch < 14; ch++) {
        if (deauth_per_channel[ch] > max_deauths) {
            max_deauths = deauth_per_channel[ch];
            most_active_channel = ch;
        }
    }
    
    if (most_active_channel > 0) {
        Serial.printf("Most active channel: %d (%lu deauth packets)\n", 
                     most_active_channel, max_deauths);
    }
    
    Serial.println("===============================\n");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
   
    Serial.println("=== Starting Multi-Channel Deauth Detection ===");
   
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
   
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_set_promiscuous_rx_cb(&packetSniffer);
    esp_wifi_set_promiscuous(true);
    
    current_channel = 1;
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    last_channel_switch = millis();
   
    total_deauth_packets = 0;
    attack_count = 0;
    last_stats_print = millis();
    full_scan_completed = false;
    scan_cycles = 0;
    
    for (int i = 0; i <= 13; i++) {
        deauth_per_channel[i] = 0;
        attacks_per_channel[i] = 0;
    }
   
    Serial.printf("Starting monitoring on channel %d...\n", current_channel);
    Serial.printf("Channel switch interval: %lu seconds\n", CHANNEL_SWITCH_INTERVAL/1000);
    Serial.println("Monitoring all WiFi channels for deauth attacks...");
}

void loop() {
    if (millis() - last_channel_switch > CHANNEL_SWITCH_INTERVAL) {
        switchChannel();
        
        if (full_scan_completed && current_channel == 1) {
            printChannelStats();
            full_scan_completed = false; // Reset flag until next full cycle
        }
    }
   
    delay(100);
}
