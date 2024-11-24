#include "Adafruit_TinyUSB.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración Wifi
#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
WiFiUDP udp;
static char udp_buf[UDP_TX_PACKET_MAX_SIZE + 1];

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
  int len = udp.parsePacket();
  if (len) {
    int n = udp.read(udp_buf, UDP_TX_PACKET_MAX_SIZE);
    udp_buf[n] = 0;
    if (usb_web.connected()) {
      IPAddress remote = udp.remoteIP();
      int32_t payload_len = sizeof(remote) + n;
      usb_web.write("start", 5);
      usb_web.write(reinterpret_cast<uint8_t *>(&payload_len), sizeof(payload_len));
      usb_web.write(reinterpret_cast<uint8_t *>(&remote), sizeof(remote));
      usb_web.write(udp_buf, n);
      usb_web.write("end", 3);
      usb_web.flush();
    }
  }
}