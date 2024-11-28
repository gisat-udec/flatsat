extern "C" {
#include "ov2640.h"
}
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "mpu9250.h"
#include <BMP180I2C.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <JPEGENC.h>

#define TFT_DC 16
#define TFT_CS 17
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_RST 20
Adafruit_ST7735 *tft;

#define OV2640_SDA 4
#define OV2640_SCL 5
#define OV2640_RESET 2
#define OV2640_VSYNC 3
#define OV2640_DATA 6  // D0
#define OV2640_WIDTH QCIF_WIDTH
#define OV2640_HEIGHT QCIF_HEIGHT
static uint8_t raw_buf[OV2640_WIDTH * OV2640_HEIGHT * 2];
static struct ov2640_config config = {
  .sccb = i2c_default,
  .pin_sioc = OV2640_SCL,
  .pin_siod = OV2640_SDA,

  .pin_resetb = OV2640_RESET,
  .pin_vsync = OV2640_VSYNC,
  .pin_y2_pio_base = OV2640_DATA,

  .pio = pio0,
  .pio_sm = 0,
  .dma_channel = 0,
  .image_buf = raw_buf,
  .image_buf_size = sizeof(raw_buf),
  .pixformat = PIXFORMAT_RGB565,

  .width = OV2640_WIDTH,
  .height = OV2640_HEIGHT,
};

uint32_t frame = 0;
#define JPEG_CHUNK_SIZE 1500
JPEGENC jpg;
JPEGENCODE jpe;
static uint8_t jpeg_buf[20000];

#define SENSOR_SDA 26
#define SENSOR_SCL 27
bfs::Mpu9250 imu;
BMP180TwoWire bmp180(&Wire1, 0x77);

#define SSID "station"
#define PASSWORD "station123"
IPAddress IP(10, 0, 0, 1);
#define PORT 8888
WiFiUDP udp;

bool camera_ok = 0;
bool imu_ok = 0;
bool ambient_ok = 0;
bool wifi_ok = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("flatsat_v0");

  SPI.setMISO(TFT_DC);
  SPI.setCS(TFT_CS);
  SPI.setSCK(TFT_SCLK); 
  SPI.setMOSI(TFT_MOSI);
  tft = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
  tft->initR(INITR_BLACKTAB);
  tft->setRotation(1);
  tft->fillScreen(ST77XX_BLACK);

  tft->setCursor(0, 10);
  tft->setTextColor(ST77XX_WHITE);
  tft->setTextSize(1);
  tft->println(" flatsat v0");

  camera_ok = (ov2640_init(&config) == 9794);
  if (camera_ok) {
    tft->setTextColor(ST77XX_GREEN);
    tft->println(" CAMARA OK");
  } else {
    tft->setTextColor(ST77XX_RED);
    tft->println(" CAMARA ERROR");
  }

  Wire1.setSDA(SENSOR_SDA);
  Wire1.setSCL(SENSOR_SCL);
  Wire1.begin();
  Wire1.setClock(400000);
  imu.Config(&Wire1, bfs::Mpu9250::I2C_ADDR_PRIM);
  imu_ok = imu.Begin();
  if (imu_ok) {
    imu.ConfigSrd(19);
    tft->setTextColor(ST77XX_GREEN);
    tft->println(" MPU 9250 OK");
  } else {
    tft->setTextColor(ST77XX_RED);
    tft->println(" MPU9250 ERROR");
  }

  ambient_ok = bmp180.begin();
  if (ambient_ok) {
    bmp180.resetToDefaults();
    bmp180.setSamplingMode(BMP180MI::MODE_UHR);
    tft->setTextColor(ST77XX_GREEN);
    tft->println(" BMP180 OK");
  } else {
    tft->setTextColor(ST77XX_RED);
    tft->println(" BMP180 ERROR");
  }

  tft->setTextColor(ST77XX_WHITE);
  tft->println(" Conectando");
  WiFi.begin(SSID, PASSWORD);
  for (int i = 0; i < 10; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      wifi_ok = 1;
      break;
    }
    tft->print('.');
    delay(500);
  }
  if (wifi_ok) {
    tft->setTextColor(ST77XX_GREEN);
    tft->println(" WIFI CONECTADO");
  } else {
    tft->setTextColor(ST77XX_RED);
    tft->println(" WIFI ERROR");
  }
  delay(1000);

  rp2040.wdt_begin(8000);
}

void loop() {
  int rem = 0;
  uint8_t chunks = 0;
  if (camera_ok) {
    ov2640_capture_frame(&config);
    tft->drawRGBBitmap(0, 0, reinterpret_cast<uint16_t*>(&raw_buf[0]), OV2640_WIDTH, OV2640_HEIGHT);
    frame++;
    jpg.open(jpeg_buf, sizeof(jpeg_buf));
    jpg.encodeBegin(&jpe, OV2640_WIDTH, OV2640_HEIGHT, JPEGE_PIXEL_RGB565, JPEGE_SUBSAMPLE_420, JPEGE_Q_LOW);
    jpg.addFrame(&jpe, raw_buf, OV2640_WIDTH * 2);
    if (jpg.getLastError() == JPEGE_SUCCESS) {
      rem = jpg.close();
      chunks = ceil(rem / (float)JPEG_CHUNK_SIZE);
    }
  }
  float imu_x = 0;
  float imu_y = 0;
  float imu_z = 0;
  float ambient_temp = 0;
  float ambient_pressure = 0;
  if (imu_ok && imu.Read()) {
    imu_x = imu.accel_x_mps2();
    imu_y = imu.accel_y_mps2();
    imu_z = imu.accel_z_mps2();
    Serial.print(imu_x);
    Serial.print("\t");
    Serial.print(imu_y);
    Serial.print("\t");
    Serial.print(imu_z);
    Serial.print("\n");
  }
  if (ambient_ok) {
    bmp180.measureTemperature();
    while (!bmp180.hasValue());
    ambient_temp = bmp180.getTemperature();
    Serial.print(ambient_temp);
    Serial.print("\t");
    bmp180.measurePressure();
    while (!bmp180.hasValue());
    ambient_pressure = bmp180.getPressure();
    Serial.print(ambient_pressure);
    Serial.print("\n");
  }
  if (wifi_ok) {
    uint8_t chunk = 0;
    while (1) {
      udp.beginPacket(IP, PORT);
      udp.write(camera_ok);
      udp.write(imu_ok);
      udp.write(ambient_ok);
      udp.write(reinterpret_cast<uint8_t*>(&imu_x), sizeof(imu_x));
      udp.write(reinterpret_cast<uint8_t*>(&imu_y), sizeof(imu_y));
      udp.write(reinterpret_cast<uint8_t*>(&imu_z), sizeof(imu_z));
      udp.write(reinterpret_cast<uint8_t*>(&ambient_temp), sizeof(ambient_temp));
      udp.write(reinterpret_cast<uint8_t*>(&ambient_pressure), sizeof(ambient_pressure));
      if (chunks != 0) {
        udp.write(uint8_t(1));
        udp.write(reinterpret_cast<uint8_t*>(&frame), sizeof(frame));
        udp.write(chunk);
        udp.write(chunks);
        int len = min(JPEG_CHUNK_SIZE, rem);
        Serial.println(len);
        udp.write(&jpeg_buf[chunk*JPEG_CHUNK_SIZE], len);
        rem -= len;
        chunk++;
      } else {
        udp.write((uint8_t)0);
      }
      udp.endPacket();
      if (chunk == chunks) {break;}
    }
  }
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  rp2040.wdt_reset();
  delay(100);
}
