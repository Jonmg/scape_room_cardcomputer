/**
 * L2_Teclado — Escribe y suena
 * ---------------------------------------------------------------
 * Placa:  M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: solo la placa. Nada conectado.
 *
 * Origen: adaptado de los ejemplos oficiales de M5Stack
 *         M5Cardputer/examples/Basic/keyboard/inputText/inputText.ino
 *         M5Cardputer/examples/Basic/buzzer/buzzer.ino
 *         (SeanKwok, shaoxiang@m5stack.com, 2023) — MIT
 *
 * QUE APRENDEMOS
 *   - ENTRADA (teclado) -> PROCESO -> SALIDA (pantalla + altavoz).
 *   - Que loop() pregunta "ha cambiado algo?" miles de veces por segundo.
 *
 * COMO SE USA
 *   Escribe letras. Cada tecla suena distinto.
 *   ENTER = envia la linea.   BACKSPACE (del) = borra.
 * ---------------------------------------------------------------
 */

#include "M5Cardputer.h"

// ===== 🔧 CAMBIA ESTO =====================================
const int  NOTA_GRAVE = 300;    // Hz de la tecla mas grave
const int  NOTA_AGUDA = 2000;   // Hz de la tecla mas aguda
const int  VOLUMEN    = 80;     // 0 a 255. Ojo con los oidos :)
// ==========================================================

M5Canvas historial(&M5Cardputer.Display);
String linea = "> ";

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);          // true = enciende el teclado

    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);

    M5Cardputer.Speaker.setVolume(VOLUMEN);

    // El "historial" es un lienzo aparte que hace scroll solo.
    historial.setTextFont(&fonts::Font0);
    historial.setTextSize(2);
    historial.createSprite(M5Cardputer.Display.width(),
                           M5Cardputer.Display.height() - 24);
    historial.setTextScroll(true);
    historial.setTextColor(GREEN);
    historial.println("Escribe algo y pulsa ENTER");
    historial.pushSprite(0, 0);

    dibujaLinea();
}

void dibujaLinea() {
    int y = M5Cardputer.Display.height() - 22;
    M5Cardputer.Display.fillRect(0, y, M5Cardputer.Display.width(), 22, BLACK);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.drawString(linea, 2, y + 2);
}

void loop() {
    M5Cardputer.update();

    if (!M5Cardputer.Keyboard.isChange()) return;   // nada nuevo, salimos
    if (!M5Cardputer.Keyboard.isPressed()) return;  // era una tecla soltandose

    Keyboard_Class::KeysState estado = M5Cardputer.Keyboard.keysState();

    for (auto c : estado.word) {
        linea += c;
        // Cada letra suena distinto: 'a' grave, 'z' aguda.
        int nota = map(c, 32, 126, NOTA_GRAVE, NOTA_AGUDA);
        M5Cardputer.Speaker.tone(nota, 40);
    }

    if (estado.del && linea.length() > 2) {
        linea.remove(linea.length() - 1);
        M5Cardputer.Speaker.tone(150, 30);
    }

    if (estado.enter) {
        historial.println(linea.substring(2));   // quitamos el "> "
        historial.pushSprite(0, 0);
        linea = "> ";
        M5Cardputer.Speaker.tone(1200, 60);
    }

    dibujaLinea();
}
