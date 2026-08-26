/**
 * L3_HexArcoiris — 37 luces de colores
 * ---------------------------------------------------------------
 * Placa:  M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit HEX (NeoHEX, 37 LEDs WS2812)
 *
 * CONEXION
 *   Unit HEX -> Port A (el conector Grove de la placa)
 *   El cable Grove lleva:  negro=GND  rojo=5V  amarillo=G2  blanco=G1
 *   Los datos de los LEDs viajan por el AMARILLO = G2.
 *
 * ⚠️ IMPORTANTE — POR QUE EL BRILLO ES BAJO
 *   El fabricante mide unos 568 mA con los 37 LEDs a blanco pleno.
 *   Con BRILLO = 30 reducimos mucho ese consumo y evitamos exigir
 *   innecesariamente a la bateria del Cardputer.
 *   Es la primera leccion de electronica: el software tiene
 *   consecuencias fisicas.
 *
 * QUE APRENDEMOS
 *   - Un color = 3 numeros: Rojo, Verde, Azul (0 a 255 cada uno).
 *   - Un bucle "for" para recorrer los 37 LEDs uno a uno.
 *
 * COMO SE USA
 *   Teclas 1..4 cambian de animacion. Flechas arriba/abajo: brillo.
 * ---------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Adafruit_NeoPixel.h>

// ===== 🔧 CAMBIA ESTO =====================================
#define PIN_HEX     2      // G2 = pin amarillo del Grove (Port A)
#define NUM_LEDS   37      // el Unit HEX tiene 37 LEDs
int BRILLO       = 30;     // 0..255.  NO subir de 60 con bateria.
// ==========================================================

Adafruit_NeoPixel hex(NUM_LEDS, PIN_HEX, NEO_GRB + NEO_KHZ800);

int modo = 1;
uint16_t paso = 0;

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);

    hex.begin();
    hex.setBrightness(BRILLO);
    hex.show();               // apagados al arrancar

    pinta();
}

void pinta() {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.println("UNIT HEX");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    M5Cardputer.Display.printf("Modo:   %d\n", modo);
    M5Cardputer.Display.setCursor(4, 56);
    M5Cardputer.Display.printf("Brillo: %d  \n", BRILLO);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 90);
    M5Cardputer.Display.println("1-4 modo  ;/. brillo");
}

// --- Modo 1: todos del mismo color, cambiando poco a poco -------
void modoUnico() {
    uint32_t c = hex.ColorHSV(paso * 256);
    for (int i = 0; i < NUM_LEDS; i++) hex.setPixelColor(i, c);
    hex.show();
    paso++;
    delay(20);
}

// --- Modo 2: arcoiris repartido entre los 37 LEDs ---------------
void modoArcoiris() {
    for (int i = 0; i < NUM_LEDS; i++) {
        uint16_t tono = paso * 256 + (i * 65536L / NUM_LEDS);
        hex.setPixelColor(i, hex.ColorHSV(tono));
    }
    hex.show();
    paso++;
    delay(15);
}

// --- Modo 3: una luz que da vueltas -----------------------------
void modoPersecucion() {
    hex.clear();
    int quien = paso % NUM_LEDS;
    hex.setPixelColor(quien, hex.Color(0, 255, 120));
    hex.show();
    paso++;
    delay(60);
}

// --- Modo 4: latido rojo ----------------------------------------
void modoLatido() {
    int v = (sin(paso / 12.0) + 1.0) * 127;      // 0..254 suave
    for (int i = 0; i < NUM_LEDS; i++) hex.setPixelColor(i, hex.Color(v, 0, 0));
    hex.show();
    paso++;
    delay(20);
}

void loop() {
    M5Cardputer.update();

    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto estado = M5Cardputer.Keyboard.keysState();
        for (auto c : estado.word) {
            if (c >= '1' && c <= '4') { modo = c - '0'; paso = 0; }
            if (c == '.') BRILLO = min(60,  BRILLO + 5);   // sube
            if (c == ';') BRILLO = max(0,   BRILLO - 5);   // baja
        }
        hex.setBrightness(BRILLO);
        pinta();
    }

    switch (modo) {
        case 1: modoUnico();       break;
        case 2: modoArcoiris();    break;
        case 3: modoPersecucion(); break;
        case 4: modoLatido();      break;
    }
}
