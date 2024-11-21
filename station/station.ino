#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración Wi-Fi
#define SSID "station"
#define PASSWORD "station123"
IPAddress apIP(10, 0, 0, 1);

// Servidor UDP
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
WiFiUDP Udp;
unsigned int localPort = 8888;

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID, PASSWORD);
  Udp.begin(localPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d)\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort(), Udp.destinationIP().toString().c_str(), Udp.localPort());

    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);
  }
}
