#include "Adafruit_TinyUSB.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración Wifi
#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
WiFiUDP udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];

// Configuración USB
Adafruit_USBD_WebUSB usb_web;
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "example.tinyusb.org/webusb-serial/index.html");

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID, PASSWORD);
  udp.begin(PORT);
  usb_web.setLandingPage(&landingPage);
  usb_web.begin();
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
  while (!TinyUSBDevice.mounted()) delay(1);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // read the packet into packetBufffer
    int n = udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    if (usb_web.connected()) {
      usb_web.write("HOLA", 4);
      usb_web.flush();
    }
  }
}