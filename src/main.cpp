#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pines específicos para XIAO ESP32-S3
#define TOUCH_PIN D1 // Pin físico D1 (GPIO 2)
#define SDA_PIN   D4 // Pin físico D4 (GPIO 5)
#define SCL_PIN   D5 // Pin físico D5 (GPIO 6)

int estadoAnimo = 0;
const int TOTAL_ESTADOS = 5;

bool estadoAnteriorTouch = LOW;
unsigned long ultimoTiempoToque = 0;
const unsigned long tiempoDebounce = 250; // ms de espera antirrebote

// Prototipos de función
void mostrarCarita(int estado);
void dibujarCorazon(int x, int y);

void setup() {
  Serial.begin(115200); // Permite ver mensajes de depuración en el Monitor Serie
  
  pinMode(TOUCH_PIN, INPUT);
  
  // Inicialización I2C con los pines de la XIAO ESP32-S3
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializar pantalla OLED (Dirección 0x3C habitual)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error: No se encontro la pantalla OLED SSD1306"));
    for(;;);
  }

  Serial.println(F("Sistema de broche iniciado."));
  display.clearDisplay();
  mostrarCarita(estadoAnimo);
}

void loop() {
  bool lecturaTouch = digitalRead(TOUCH_PIN);

  // Detección de toque (flanco de subida LOW a HIGH)
  if (lecturaTouch == HIGH && estadoAnteriorTouch == LOW) {
    if (millis() - ultimoTiempoToque > tiempoDebounce) {
      estadoAnimo = (estadoAnimo + 1) % TOTAL_ESTADOS;
      
      Serial.print("Toque detectado. Nuevo estado: ");
      Serial.println(estadoAnimo);

      mostrarCarita(estadoAnimo);
      ultimoTiempoToque = millis();
    }
  }
  
  estadoAnteriorTouch = lecturaTouch;
  delay(10);
}

// Dibujo gráfico de expresiones corregidas y alineadas
void mostrarCarita(int estado) {
  display.clearDisplay();
  
  switch(estado) {
    case 0: // Feliz
      // Ojos con brillo interno
      display.fillCircle(35, 22, 10, SSD1306_WHITE);
      display.fillCircle(93, 22, 10, SSD1306_WHITE);
      display.fillCircle(38, 19, 4, SSD1306_BLACK); 
      display.fillCircle(96, 19, 4, SSD1306_BLACK); 

      // Sonrisa limpia sin invadir la zona de los ojos (x de 46 a 82)
      display.drawCircle(64, 35, 17, SSD1306_WHITE);
      display.drawCircle(64, 35, 16, SSD1306_WHITE);
      display.fillRect(46, 18, 36, 17, SSD1306_BLACK);
      break;

    case 1: // Sorprendido
      // Ojos bien abiertos
      display.drawCircle(35, 22, 12, SSD1306_WHITE);
      display.drawCircle(93, 22, 12, SSD1306_WHITE);
      display.fillCircle(35, 22, 5, SSD1306_WHITE);
      display.fillCircle(93, 22, 5, SSD1306_WHITE);
      
      // Cejas elevadas
      display.drawFastHLine(23, 7, 24, SSD1306_WHITE);
      display.drawFastHLine(81, 7, 24, SSD1306_WHITE);

      // Boca en "O"
      display.fillCircle(64, 48, 8, SSD1306_WHITE);
      display.fillCircle(64, 48, 4, SSD1306_BLACK);
      break;

    case 2: // Enojado
      // Ojos
      display.fillCircle(35, 26, 8, SSD1306_WHITE);
      display.fillCircle(93, 26, 8, SSD1306_WHITE);

      // Cejas inclinadas
      display.drawLine(20, 10, 48, 22, SSD1306_WHITE);
      display.drawLine(20, 11, 48, 23, SSD1306_WHITE);
      display.drawLine(108, 10, 80, 22, SSD1306_WHITE);
      display.drawLine(108, 11, 80, 23, SSD1306_WHITE);

      // Boca seria / comisuras hacia abajo
      display.drawLine(48, 50, 80, 50, SSD1306_WHITE);
      display.drawLine(48, 50, 43, 54, SSD1306_WHITE);
      display.drawLine(80, 50, 85, 54, SSD1306_WHITE);
      break;

    case 3: // Guiño / Coqueta
      // Ojo izquierdo guiñado (forma de arco feliz "^")
      display.drawLine(25, 24, 35, 17, SSD1306_WHITE);
      display.drawLine(35, 17, 45, 24, SSD1306_WHITE);
      display.drawLine(25, 25, 35, 18, SSD1306_WHITE);
      display.drawLine(35, 18, 45, 25, SSD1306_WHITE);

      // Ojo derecho abierto
      display.fillCircle(93, 22, 10, SSD1306_WHITE);
      display.fillCircle(96, 19, 4, SSD1306_BLACK);

      // Sonrisa ladeada
      display.drawCircle(68, 38, 14, SSD1306_WHITE);
      display.fillRect(50, 22, 36, 16, SSD1306_BLACK);
      break;

    case 4: // Enamorado
      // Ojos en forma de corazón
      dibujarCorazon(35, 22);
      dibujarCorazon(93, 22);

      // Gran sonrisa
      display.drawCircle(64, 36, 16, SSD1306_WHITE);
      display.fillRect(46, 18, 36, 18, SSD1306_BLACK);
      break;
  }
  
  display.display();
}

// Función para dibujar corazones proporcionales
void dibujarCorazon(int x, int y) {
  display.fillCircle(x - 5, y - 3, 5, SSD1306_WHITE);
  display.fillCircle(x + 5, y - 3, 5, SSD1306_WHITE);
  display.fillTriangle(x - 10, y - 1, x + 10, y - 1, x, y + 10, SSD1306_WHITE);
}