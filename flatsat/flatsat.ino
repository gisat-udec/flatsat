#include "pico/stdlib.h"
extern "C" {
#include "ov2640.h"
}
#include "tusb.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <JPEGENC.h>

// * Pines*
// - Camara OV2640 red board
#define SDA 4
#define SCL 5
#define OV2640_RESET 2
#define OV2640_VSYNC 3
// Los pines D0, D1, D2, D3, D4, D5, D6, DCLK, HREF deben estar en orden
// En este caso desde GPIO 6 hasta GPIO 15
#define OV2640_DATA 6  // D0
// - Pantalla ST7735S
#define TFT_DC 16
#define TFT_CS 17
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_RST 20

// Configuración de la camara
uint32_t frame = 0;
#define JPEG_CHUNK_SIZE 2000
JPEGENC jpg;
JPEGENCODE jpe;
static uint8_t jpeg_buf[20000];
#define TFT_WIDTH 128
#define TFT_HEIGHT 160
#define WIDTH QCIF_WIDTH
#define HEIGHT QCIF_HEIGHT
static uint8_t raw_buf[WIDTH * HEIGHT * 2];
static struct ov2640_config config = {
  .sccb = i2c_default,
  .pin_sioc = SCL,
  .pin_siod = SDA,

  .pin_resetb = OV2640_RESET,
  .pin_vsync = OV2640_VSYNC,
  .pin_y2_pio_base = OV2640_DATA,

  .pio = pio0,
  .pio_sm = 0,
  .dma_channel = 0,
  .image_buf = raw_buf,
  .image_buf_size = sizeof(raw_buf),
  .pixformat = PIXFORMAT_RGB565,

  .width = WIDTH,
  .height = HEIGHT,
};

// Configuración de la pantalla
Adafruit_ST7735 *tft;

// Configuración Wifi
#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
WiFiUDP udp;

enum {
  PACKET_JPEG = 0
};

void setup() {
  // WiFi.begin(SSID, PASSWORD);
  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.print('.');
  //   delay(500);
  // }
  // Serial.print("Connected! IP address: ");
  // Serial.println(WiFi.localIP());
  ov2640_init(&config);

  SPI.setMISO(TFT_DC);
  SPI.setCS(TFT_CS);
  SPI.setSCK(TFT_SCLK); 
  SPI.setMOSI(TFT_MOSI);
  tft = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
  tft->initR(INITR_GREENTAB);
  tft->setRotation(1);
  tft->fillScreen(ST77XX_CYAN);
}

void loop() {
  // Capturar
  ov2640_capture_frame(&config);
  tft->drawRGBBitmap(0, 0, reinterpret_cast<uint16_t*>(&raw_buf[0]), WIDTH, HEIGHT);
  frame++;
  jpg.open(jpeg_buf, sizeof(jpeg_buf));
  jpg.encodeBegin(&jpe, WIDTH, HEIGHT, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_LOW);
  jpg.addFrame(&jpe, raw_buf, WIDTH * 2);

  // Enviar
  if (jpg.getLastError() == JPEGE_SUCCESS) {
    int rem = jpg.close();
    printf("Enviando jpeg bytes %d\n", rem);
    uint8_t total_chunks = ceil(rem / (float)JPEG_CHUNK_SIZE);
    uint8_t chunk = 0;
    // while (rem > 0) {
    //   udp.beginPacket(IP, PORT);
    //   udp.write(PACKET_JPEG);
    //   udp.write(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
    //   udp.write(chunk);
    //   udp.write(total_chunks);
    //   int chunk_size = min(JPEG_CHUNK_SIZE, rem);
    //   udp.write(&jpeg_buf[chunk*JPEG_CHUNK_SIZE], chunk_size);
    //   if (udp.endPacket()) {
    //     rem -= chunk_size;
    //     chunk++;
    //   }
    //  }
    printf("\n");
  }

  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
