/**
 * L1_Hola — "Hola, me llamo..."
 * ---------------------------------------------------------------
 * Placa:  M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: solo la placa. Nada conectado.
 *
 * Origen: adaptado del ejemplo oficial de M5Stack
 *         M5Cardputer/examples/Basic/display/display.ino
 *         (SeanKwok, shaoxiang@m5stack.com, 2023) — MIT
 *
 * QUE APRENDEMOS
 *   - Que un programa tiene dos partes: setup() se ejecuta UNA vez,
 *     loop() se repite para siempre.
 *   - Que la pantalla se dibuja con coordenadas (x, y).
 * ---------------------------------------------------------------
 */

#include "M5Cardputer.h"

// ===== 🔧 CAMBIA ESTO =====================================
const char* MI_NOMBRE = "Jon";       // <- pon tu nombre aqui
uint16_t COLOR_TEXTO  = GREEN;       // GREEN, RED, BLUE, YELLOW, WHITE, ORANGE...
uint16_t COLOR_FONDO  = BLACK;
// ==========================================================

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);          // pantalla en horizontal
    M5Cardputer.Display.fillScreen(COLOR_FONDO);

    M5Cardputer.Display.setTextColor(COLOR_TEXTO, COLOR_FONDO);
    M5Cardputer.Display.setTextDatum(middle_center);   // texto centrado
    M5Cardputer.Display.setTextFont(&fonts::FreeSansBold18pt7b);

    int centroX = M5Cardputer.Display.width()  / 2;
    int centroY = M5Cardputer.Display.height() / 2;

    M5Cardputer.Display.drawString("Hola,", centroX, centroY - 25);
    M5Cardputer.Display.drawString(MI_NOMBRE,  centroX, centroY + 15);
}

void loop() {
    M5Cardputer.update();

    // Abajo a la izquierda mostramos la bateria, y se refresca sola.
    // Es la prueba de que loop() se esta repitiendo sin parar.
    M5Cardputer.Display.setTextDatum(bottom_left);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.drawString(
        String("Bateria: ") + M5Cardputer.Power.getBatteryLevel() + "%   ",
        4, M5Cardputer.Display.height() - 4);

    delay(1000);   // esperamos 1 segundo (1000 milisegundos)
}
