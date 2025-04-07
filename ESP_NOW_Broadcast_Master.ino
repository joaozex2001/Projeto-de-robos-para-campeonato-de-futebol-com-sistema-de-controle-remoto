/*
    ESP-NOW Broadcast Master
    Lucas Saavedra Vaz - 2024

    This sketch demonstrates how to broadcast messages to all devices within the ESP-NOW network.
    This example is intended to be used with the ESP-NOW Broadcast Slave example.

    The master device will broadcast a message every 5 seconds to all devices within the network.
    This will be done using by registering a peer object with the broadcast address.

    The slave devices will receive the broadcasted messages and print them to the Serial Monitor.
*/

#include "ESP32_NOW.h"
#include "WiFi.h"

#include <esp_mac.h>  // For the MAC2STR and MACSTR macros
#include <math.h>

#define VERBOSE_MODE  1

#define CONTROLLER_0  32
#define CONTROLLER_1  33
#define CONTROLLER_2  25
#define CONTROLLER_3  26

#define BTN_0  36
#define BTN_1  39
#define BTN_2  34
#define BTN_3  35

/* Definitions */

#define ESPNOW_WIFI_CHANNEL 6

char data_to_send[17] = "111111111111";
const int btn_seq[4] = { BTN_0, BTN_1, BTN_2, BTN_3 };

/* Classes */

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

bool scan_controllers() {
  
  int i,j,k = 0;
  char b;
  bool r = false; 

  for(i=0; i<4; ++i) {
    digitalWrite(CONTROLLER_0,i!=0);
    digitalWrite(CONTROLLER_1,i!=1);
    digitalWrite(CONTROLLER_2,i!=2);
    digitalWrite(CONTROLLER_3,i!=3);
    delay(10);
    for(j=0; j<4; ++j) {
      b = digitalRead(btn_seq[j]) ? '1' : '0';
      if(b!=data_to_send[k]) {
        r = true;
      }
      data_to_send[k] = b;
      k++;
    }
  }
  return r;
}

/* Global Variables */

// uint32_t msg_count = 0;

// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

/* Main */

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  pinMode(CONTROLLER_0,OUTPUT);
  pinMode(CONTROLLER_1,OUTPUT);
  pinMode(CONTROLLER_2,OUTPUT);
  pinMode(CONTROLLER_3,OUTPUT);

  digitalWrite(CONTROLLER_0,HIGH);
  digitalWrite(CONTROLLER_1,HIGH);
  digitalWrite(CONTROLLER_2,HIGH);
  digitalWrite(CONTROLLER_3,HIGH);

  pinMode(BTN_0,INPUT);
  pinMode(BTN_1,INPUT);
  pinMode(BTN_2,INPUT);
  pinMode(BTN_3,INPUT);

  data_to_send[16] = '\0';

  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
}

void loop() {

  if(scan_controllers()) {
    if(VERBOSE_MODE) {
      Serial.printf(" %s\n", data_to_send);
    }
    if (!broadcast_peer.send_message((uint8_t *)data_to_send, sizeof(data_to_send))) {
        Serial.println("Failed to broadcast message");
    }
  }

}
