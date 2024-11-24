#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración Wifi
#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
WiFiUDP udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID, PASSWORD);
  udp.begin(PORT);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    int n = udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);
  }
}