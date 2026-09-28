#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "memes.h"

// Pines I2C para ESP32-S3 (Ajusta según la asignación de tu placa)
#define SDA_PIN 8
#define SCL_PIN 9

// Configuración del Touch
// Opción A: Módulo digital TTP223 conectado a un GPIO
// Opción B: Pin Capacitivo nativo ESP32 (ej. TOUCH1 / GPIO1)
#define TOUCH_PIN 4
#define USE_DIGITAL_TOUCH  true// Cambia a 'false' si usas touchRead() capacitivo nativo
#define TOUCH_THRESHOLD 30000  // Umbral de disparo para touchRead() en ESP32-S3

// Pantalla OLED SSD1306 (128x64)
#define OLED_RESET -1
Adafruit_SSD1306 display(MemeAssets::SCREEN_WIDTH, MemeAssets::SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables de estado y control
uint8_t currentMemeIndex = 0;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
constexpr unsigned long DEBOUNCE_DELAY = 200; // ms

void showMeme(uint8_t index) {
    display.clearDisplay();

    const MemeAssets::MemeBitmap& meme = MemeAssets::MEME_LIST[index];
    uint16_t drawWidth;
    uint16_t drawHeight;

    if (static_cast<uint32_t>(MemeAssets::SCREEN_WIDTH) * meme.height <=
        static_cast<uint32_t>(MemeAssets::SCREEN_HEIGHT) * meme.width) {
        drawWidth = MemeAssets::SCREEN_WIDTH;
        drawHeight = static_cast<uint32_t>(meme.height) * drawWidth / meme.width;
    } else {
        drawHeight = MemeAssets::SCREEN_HEIGHT;
        drawWidth = static_cast<uint32_t>(meme.width) * drawHeight / meme.height;
    }

    const uint16_t xOffset = (MemeAssets::SCREEN_WIDTH - drawWidth) / 2;
    const uint16_t yOffset = (MemeAssets::SCREEN_HEIGHT - drawHeight) / 2;
    const uint16_t sourceStride = (meme.width + 7) / 8;

    for (uint16_t y = 0; y < drawHeight; ++y) {
        const uint16_t sourceY = static_cast<uint32_t>(2 * y + 1) * meme.height / (2 * drawHeight);
        for (uint16_t x = 0; x < drawWidth; ++x) {
            const uint16_t sourceX = static_cast<uint32_t>(2 * x + 1) * meme.width / (2 * drawWidth);
            const uint8_t sourceByte = pgm_read_byte(&meme.data[sourceY * sourceStride + sourceX / 8]);
            if (sourceByte & (0x80 >> (sourceX & 7))) {
                display.drawPixel(xOffset + x, yOffset + y, SSD1306_WHITE);
            }
        }
    }

    display.display();
}

bool checkTouch() {
#if USE_DIGITAL_TOUCH
    return digitalRead(TOUCH_PIN) == HIGH;
#else
    return touchRead(TOUCH_PIN) > TOUCH_THRESHOLD;
#endif
}

void setup() {
    Serial.begin(115200);

    // Configuración del sensor Touch
    pinMode(TOUCH_PIN, INPUT);

    // Inicialización del bus I2C y la Pantalla OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Dirección I2C común: 0x3C
        Serial.println(F("Error: No se encontró la pantalla OLED SSD1306"));
        for (;;);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 25);
    display.println(F("Toca para Meme..."));
    display.display();
    delay(1000);

    // Muestra el primer meme
    showMeme(currentMemeIndex);
}

void loop() {
    bool rawTouch = checkTouch();

    // Detección de flanco de subida con debounce
    if (rawTouch && !lastTouchState && (millis() - lastDebounceTime > DEBOUNCE_DELAY)) {
        lastDebounceTime = millis();

        // Siguiente meme en el arreglo circular
        currentMemeIndex = (currentMemeIndex + 1) % MemeAssets::TOTAL_MEMES;
        Serial.printf("Cambiando a meme indice: %d\n", currentMemeIndex);

        showMeme(currentMemeIndex);
    }

    lastTouchState = rawTouch;
}
