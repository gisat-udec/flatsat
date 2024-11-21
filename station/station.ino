#include <WiFi.h>
#include <WiFiAP.h>
#include <NetworkUdp.h>

// Configuración Wi-Fi
#define SSID "station"
#define PASSWORD "station123"
IPAddress apIP(10, 0, 0, 1);

// Configuración servidor UDP
NetworkUDP udp;
char packetBuffer[65565];
unsigned int localPort = 8888;

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID, PASSWORD);
    udp.begin(localPort);
}

void loop() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int n = udp.read(packetBuffer, sizeof(packetBuffer));
        packetBuffer[n] = 0;
        Serial.println(packetBuffer);
    }
}
