#include "pico/stdlib.h"
extern "C" {
  #include "ov2640.h"
} 

// Pines
#define SDA 4
#define SCL 5
#define OV2640_RESET 2
#define OV2640_VSYNC 3
#define OV2640_DATA 6 // D0; deben estar en orden D0, D1, D2, D3, D4, D5, D6, DCLK, HREF

// Configuración de la camara
static uint8_t image_buf[320*240*2];
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
    .image_buf = image_buf,
    .image_buf_size = sizeof(image_buf),
    .pixformat = PIXFORMAT_RGB565,
};

void setup() {
  Serial.begin(115200);
  ov2640_init(&config);
}

void loop() {
  Serial.println("capture frame");
  ov2640_capture_frame(&config);
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  delay(500);
}
