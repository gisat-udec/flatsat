#include "Adafruit_TinyUSB.h"
#include <FreeRTOS.h>
#include <queue.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Configuración Wifi
#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
#define UDP_MAX 3000
WiFiUDP udp;
struct packet {
  int32_t len;
  IPAddress ip;
  uint8_t udp_buf[UDP_MAX];
} packet;

// Configuración USB
Adafruit_USBD_WebUSB usb_web;
WEBUSB_URL_DEF(landingPage, 1 /*https*/, "example.tinyusb.org/webusb-serial/index.html");

// Configuracion Queue
#define QUEUE_ITEMS 50
static StaticQueue_t xStaticQueue;
uint8_t queue_buf[sizeof(packet) * QUEUE_ITEMS];
QueueHandle_t xQueue = NULL;

// Core 0 encargado de recibir datos por wifi y cargarlos a la RAM
void setup() {
  xQueue = xQueueCreateStatic(QUEUE_ITEMS, sizeof(packet), queue_buf, &xStaticQueue);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP, IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(SSID, PASSWORD);
  udp.begin(PORT);
}

void loop() {
  int len = udp.parsePacket();
  if (len) {
    int n = udp.read(packet.udp_buf, sizeof(packet.udp_buf));
    packet.ip = udp.remoteIP();
    packet.len = sizeof(packet.len) + sizeof(packet.ip) + n;
    if (xQueue != NULL) {
      xQueueSendToBack(xQueue, &packet, 0);
    }
  }
}

// Core 1 encargado de transmitir desde RAM a WebUSB
void setup1() {
  usb_web.setLandingPage(&landingPage);
  usb_web.begin();
  if (TinyUSBDevice.mounted()) {
  TinyUSBDevice.detach();
  delay(10);
  TinyUSBDevice.attach();
  }
  while (!TinyUSBDevice.mounted()) delay(1);
}

void loop1() {
  if (xQueue != NULL) {
    struct packet to_send;
    while (xQueueReceive(xQueue, &to_send, 0) == pdTRUE) {
      if (usb_web.connected()) {
        usb_web.write("start", 5);
        usb_web.write(reinterpret_cast<uint8_t*>(&to_send), to_send.len);
        usb_web.write("end", 3);
      }
    }
    if (usb_web.connected()) {
      usb_web.flush();
    }
  }
  delay(100);
}