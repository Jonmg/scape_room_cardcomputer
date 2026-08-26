/**
 * E1_EscapeRoom — El laboratorio de las luces
 * ------------------------------------------------------------------
 * Placa: M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit HEX + Unit RFID2 + 3 tarjetas RFID
 *
 * MONTAJE ACTUAL — LOS DOS MODULOS A LA VEZ
 *   RFID2 -> Port A.
 *   HEX   -> G4 + GND + 5VOUT del conector EXT.
 *   Se resuelve el codigo de colores y el juego pasa directamente
 *   a las tres escenas RFID, sin apagar ni cambiar cables.
 *
 * ALTERNATIVA SIN CABLE EXT
 *   Cambia CAMBIO_MANUAL_MODULOS a true y PIN_HEX a 2. El juego
 *   guardara el progreso antes de pedir que se intercambien modulos.
 *
 * Origen: combina y amplia las lecciones de este repositorio,
 *         adaptadas de ejemplos oficiales de M5Stack (licencia MIT).
 * ------------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <Wire.h>
#include "MFRC522_I2C.h"

// ===== 🔧 CAMBIA ESTO =============================================

// false = RFID2 en Port A y HEX en G4 + 5VOUT + GND del EXT.
// true  = alternativa sin cable EXT: intercambiar ambos en Port A.
const bool CAMBIO_MANUAL_MODULOS = false;

#define PIN_HEX             4   // G4 del EXT; deja G2/G1 libres para el RFID2
#define NUM_LEDS           37
const int BRILLO_HEX       = 25; // mantenlo bajo funcionando con bateria

#define DIR_RFID2        0x28
#define PIN_RESET_FALSO     6
#define PIN_SDA              2
#define PIN_SCL              1

// Mientras sea true, valen tres tarjetas distintas cualesquiera.
// Sus UID aparecen en pantalla y en Serial para poder apuntarlos.
const bool MODO_DEMO_UID = true;

// Cuando tengas los UID, pegalos aqui y cambia MODO_DEMO_UID a false.
const char* UID_TARJETAS[3] = {
    "A3 2F 91 04",
    "B4 00 12 34",
    "C5 00 56 78"
};

// Lugares donde el adulto esconde cada tarjeta.
const char* PISTA_LINEA_1[3] = {
    "Busca cerca de algo",
    "Busca donde viven",
    "Busca bajo algo con"
};
const char* PISTA_LINEA_2[3] = {
    "que marca el tiempo.",
    "muchas historias.",
    "patas que no camina."
};

// Tabla de sustitucion: cada color representa un numero.
// La secuencia actual es AZUL, VERDE, ROJO, AMARILLO -> 1379.
enum ColorClave : uint8_t { ROJO, VERDE, AZUL, AMARILLO };
const ColorClave SECUENCIA_HEX[4] = { AZUL, VERDE, ROJO, AMARILLO };

// ==================================================================

struct DatosColor {
    const char* nombre;
    char digito;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t colorPantalla;
};

const DatosColor COLORES[4] = {
    { "ROJO",     '7', 255,   0,   0, RED    },
    { "VERDE",    '3',   0, 255,   0, GREEN  },
    { "AZUL",     '1',   0,  70, 255, BLUE   },
    { "AMARILLO", '9', 255, 180,   0, YELLOW }
};

enum Escena : uint8_t {
    INTRO,
    CLAVE_HEX,
    CAMBIO_A_RFID,
    TARJETA_1,
    TARJETA_2,
    TARJETA_3,
    VICTORIA
};

Adafruit_NeoPixel hex(NUM_LEDS, PIN_HEX, NEO_GRB + NEO_KHZ800);
MFRC522_I2C lector(DIR_RFID2, PIN_RESET_FALSO);
Preferences memoria;

Escena escena = INTRO;
bool hexListo = false;
bool rfidListo = false;
byte versionRfid = 0;

String codigoEscrito;
int intentosCodigo = 0;
String uidDemo[3];

int destelloActual = -1;
bool destelloEncendido = false;
unsigned long proximoDestello = 0;
uint16_t pasoVictoria = 0;
bool esperandoSoltarG0 = false;

// ------------------------------------------------------------------
// Declaraciones: hacen visible el mapa general del programa.
// ------------------------------------------------------------------
void iniciaHex();
bool iniciaRfid();
void dibujaEscena();
void cambiaEscena(Escena nueva);
void procesaTeclado();
void actualizaHex();
void actualizaRfid();
bool gestionaReinicio();

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Speaker.setVolume(70);
    Serial.begin(115200);

    memoria.begin("escape", false);
    byte escenaGuardada = memoria.getUChar("escena", INTRO);
    escena = escenaGuardada <= VICTORIA ? (Escena)escenaGuardada : INTRO;
    for (int i = 0; i < 3; i++) {
        uidDemo[i] = memoria.getString((String("uid") + i).c_str(), "");
    }

    // En el montaje doble el HEX puede estar disponible todo el juego.
    // En el montaje actual solo lo iniciamos durante sus propias escenas.
    if (!CAMBIO_MANUAL_MODULOS || escena <= CLAVE_HEX) iniciaHex();

    if (escena >= CAMBIO_A_RFID && escena <= TARJETA_3) {
        rfidListo = iniciaRfid();

        // Esta escena queda guardada justo antes de apagar y cambiar modulo.
        // Si al arrancar ya vemos el RFID2, continuamos automaticamente.
        if (escena == CAMBIO_A_RFID && rfidListo) {
            cambiaEscena(TARJETA_1);
            return;
        }
    }

    dibujaEscena();
}

void loop() {
    M5Cardputer.update();

    if (gestionaReinicio()) return;

    procesaTeclado();

    if (escena == CLAVE_HEX) actualizaHex();
    if (escena >= TARJETA_1 && escena <= TARJETA_3) actualizaRfid();

    // Con ambos modulos conectados, el HEX tambien celebra la victoria.
    if (escena == VICTORIA && !CAMBIO_MANUAL_MODULOS && hexListo) {
        for (int i = 0; i < NUM_LEDS; i++) {
            uint16_t tono = pasoVictoria * 256 + i * (65536L / NUM_LEDS);
            hex.setPixelColor(i, hex.ColorHSV(tono));
        }
        hex.show();
        pasoVictoria++;
        delay(15);
    }
}

// ------------------------------------------------------------------
// Hardware: cada modulo tiene su propia funcion de preparacion.
// ------------------------------------------------------------------
void iniciaHex() {
    hex.begin();
    hex.setBrightness(min(BRILLO_HEX, 60));
    hex.clear();
    hex.show();
    hexListo = true;
}

void apagaHex() {
    if (!hexListo) return;
    hex.clear();
    hex.show();
    if (CAMBIO_MANUAL_MODULOS) {
        pinMode(PIN_HEX, INPUT); // libera G2 antes de convertirlo en SDA
        hexListo = false;
    }
}

bool iniciaRfid() {
    Wire.begin(PIN_SDA, PIN_SCL, 100000);
    lector.PCD_Init();
    delay(80);
    versionRfid = lector.PCD_ReadRegister(MFRC522_I2C::VersionReg);
    Serial.printf("RFID2, version del chip: 0x%02X\n", versionRfid);
    return versionRfid != 0x00 && versionRfid != 0xFF;
}

// ------------------------------------------------------------------
// Escenas y pantalla.
// ------------------------------------------------------------------
void limpiaPantalla(uint16_t colorTitulo, const char* titulo) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(colorTitulo, BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.println(titulo);
}

void dibujaIntro() {
    limpiaPantalla(CYAN, "LABORATORIO SECRETO");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    M5Cardputer.Display.println("Las luces esconden");
    M5Cardputer.Display.println("la primera clave.");
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 86);
    M5Cardputer.Display.println("ENTER para empezar");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 120);
    M5Cardputer.Display.println("G0 durante 3 s: juego nuevo");
}

void dibujaEntradaCodigo() {
    String visible = codigoEscrito;
    while (visible.length() < 4) visible += '_';

    M5Cardputer.Display.fillRect(0, 77, 240, 26, BLACK);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 80);
    M5Cardputer.Display.print("Codigo: ");
    M5Cardputer.Display.println(visible);
}

void dibujaClaveHex() {
    limpiaPantalla(CYAN, "CLAVE DE COLORES");

    // Cada caja es el diccionario: color -> numero.
    for (int i = 0; i < 4; i++) {
        int x = 4 + i * 59;
        M5Cardputer.Display.fillRoundRect(x, 25, 53, 24, 3,
                                          COLORES[i].colorPantalla);
        M5Cardputer.Display.setTextDatum(middle_center);
        M5Cardputer.Display.setTextColor(i == AMARILLO ? BLACK : WHITE,
                                         COLORES[i].colorPantalla);
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.drawString(String(COLORES[i].digito), x + 26, 37);
    }

    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 57);
    M5Cardputer.Display.println("Sustituye los 4 destellos por numeros");
    dibujaEntradaCodigo();
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 112);
    M5Cardputer.Display.println("ENTER comprueba   DEL borra");
}

void dibujaCambioRfid() {
    limpiaPantalla(YELLOW, "CAMBIO DE MODULO");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 31);

    if (CAMBIO_MANUAL_MODULOS) {
        M5Cardputer.Display.println("1. Desconecta el cable USB");
        M5Cardputer.Display.println("2. Pon el interruptor en OFF");
        M5Cardputer.Display.println("3. Cambia HEX por RFID2");
        M5Cardputer.Display.println("4. Port A en 5VOUT");
        M5Cardputer.Display.println("5. Pon ON y vuelve a encender");
        M5Cardputer.Display.setTextColor(GREEN, BLACK);
        M5Cardputer.Display.setCursor(4, 112);
        M5Cardputer.Display.println("El progreso ya esta guardado.");
    } else {
        M5Cardputer.Display.setTextColor(RED, BLACK);
        M5Cardputer.Display.println("NO VEO EL RFID2");
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.println("Revisa Port A y reinicia.");
    }
}

int indiceTarjetaActual() {
    return (int)escena - (int)TARJETA_1;
}

void dibujaBuscaTarjeta() {
    int i = indiceTarjetaActual();
    char titulo[24];
    snprintf(titulo, sizeof(titulo), "LLAVE RFID %d DE 3", i + 1);
    limpiaPantalla(GREEN, titulo);

    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 37);
    M5Cardputer.Display.println(PISTA_LINEA_1[i]);
    M5Cardputer.Display.println(PISTA_LINEA_2[i]);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 85);
    M5Cardputer.Display.println("Acercala al lector");

    if (MODO_DEMO_UID) {
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
        M5Cardputer.Display.setCursor(4, 119);
        M5Cardputer.Display.println("DEMO: usa 3 tarjetas diferentes");
    }
}

void dibujaVictoria() {
    limpiaPantalla(GREEN, "ESCAPE COMPLETADO!");
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setTextFont(&fonts::FreeSansBold18pt7b);
    M5Cardputer.Display.drawString("LIBRES!", 120, 68);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 121);
    M5Cardputer.Display.println("G0 durante 3 s para reiniciar");

    M5Cardputer.Speaker.tone(700, 100);  delay(120);
    M5Cardputer.Speaker.tone(1000, 100); delay(120);
    M5Cardputer.Speaker.tone(1400, 180);
}

void dibujaEscena() {
    switch (escena) {
        case INTRO:          dibujaIntro();         break;
        case CLAVE_HEX:      dibujaClaveHex();      break;
        case CAMBIO_A_RFID:  dibujaCambioRfid();    break;
        case TARJETA_1:
        case TARJETA_2:
        case TARJETA_3:
            if (rfidListo) dibujaBuscaTarjeta();
            else dibujaCambioRfid();
            break;
        case VICTORIA:       dibujaVictoria();      break;
    }
}

void cambiaEscena(Escena nueva) {
    escena = nueva;
    memoria.putUChar("escena", (byte)escena);
    dibujaEscena();
}

// ------------------------------------------------------------------
// Prueba 1: leer los destellos y escribir su traduccion.
// ------------------------------------------------------------------
String codigoCorrecto() {
    String codigo;
    for (int i = 0; i < 4; i++) codigo += COLORES[SECUENCIA_HEX[i]].digito;
    return codigo;
}

void rellenaHex(ColorClave cual) {
    const DatosColor& c = COLORES[cual];
    uint32_t color = hex.Color(c.r, c.g, c.b);
    for (int i = 0; i < NUM_LEDS; i++) hex.setPixelColor(i, color);
    hex.show();
}

void actualizaHex() {
    if (!hexListo || millis() < proximoDestello) return;

    if (destelloEncendido) {
        hex.clear();
        hex.show();
        destelloEncendido = false;
        proximoDestello = millis() + (destelloActual == 3 ? 1300 : 300);
    } else {
        destelloActual = (destelloActual + 1) % 4;
        rellenaHex(SECUENCIA_HEX[destelloActual]);
        destelloEncendido = true;
        proximoDestello = millis() + 750;
    }
}

void codigoFallido() {
    intentosCodigo++;
    M5Cardputer.Speaker.tone(180, 180);
    M5Cardputer.Display.fillRect(0, 103, 240, 28, BLACK);
    M5Cardputer.Display.setTextColor(RED, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 106);
    M5Cardputer.Display.printf("No coincide. Intento %d", intentosCodigo);
    delay(1000);
    codigoEscrito = "";
    dibujaClaveHex();
}

void codigoResuelto() {
    M5Cardputer.Speaker.tone(800, 80);  delay(100);
    M5Cardputer.Speaker.tone(1200, 80); delay(100);
    M5Cardputer.Speaker.tone(1700, 140);

    apagaHex();
    escena = CAMBIO_A_RFID;
    memoria.putUChar("escena", (byte)escena);

    if (CAMBIO_MANUAL_MODULOS) {
        dibujaCambioRfid();
        return;
    }

    rfidListo = iniciaRfid();
    if (rfidListo) cambiaEscena(TARJETA_1);
    else dibujaCambioRfid();
}

void procesaTeclado() {
    if (!M5Cardputer.Keyboard.isChange() ||
        !M5Cardputer.Keyboard.isPressed()) return;

    auto teclas = M5Cardputer.Keyboard.keysState();

    if (escena == INTRO && teclas.enter) {
        cambiaEscena(CLAVE_HEX);
        destelloActual = -1;
        destelloEncendido = false;
        proximoDestello = 0;
        return;
    }

    if (escena != CLAVE_HEX) return;

    for (auto c : teclas.word) {
        if (c >= '0' && c <= '9' && codigoEscrito.length() < 4) {
            codigoEscrito += c;
            M5Cardputer.Speaker.tone(500 + (c - '0') * 80, 25);
        }
    }

    if (teclas.del && codigoEscrito.length() > 0) {
        codigoEscrito.remove(codigoEscrito.length() - 1);
        M5Cardputer.Speaker.tone(220, 25);
    }

    dibujaEntradaCodigo();

    if (teclas.enter) {
        if (codigoEscrito == codigoCorrecto()) codigoResuelto();
        else codigoFallido();
    }
}

// ------------------------------------------------------------------
// Prueba 2: encontrar y presentar las tres tarjetas en orden.
// ------------------------------------------------------------------
String uidComoTexto() {
    String s;
    for (byte i = 0; i < lector.uid.size; i++) {
        if (lector.uid.uidByte[i] < 0x10) s += '0';
        s += String(lector.uid.uidByte[i], HEX);
        if (i < lector.uid.size - 1) s += ' ';
    }
    s.toUpperCase();
    return s;
}

bool uidYaUsado(const String& uid) {
    for (int i = 0; i < 3; i++) {
        if (uidDemo[i].length() && uidDemo[i] == uid) return true;
    }
    return false;
}

void muestraResultadoTarjeta(bool correcta, const String& uid,
                              const char* mensaje) {
    limpiaPantalla(correcta ? GREEN : RED,
                   correcta ? "LLAVE CORRECTA" : "ESA NO SIRVE");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 38);
    M5Cardputer.Display.println(mensaje);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 80);
    M5Cardputer.Display.println(uid);
    M5Cardputer.Speaker.tone(correcta ? 1500 : 180, correcta ? 100 : 220);
}

void actualizaRfid() {
    if (!rfidListo) return;
    if (!lector.PICC_IsNewCardPresent()) { delay(35); return; }
    if (!lector.PICC_ReadCardSerial())   { delay(35); return; }

    String uid = uidComoTexto();
    int i = indiceTarjetaActual();
    Serial.printf("Tarjeta %d -> UID: %s\n", i + 1, uid.c_str());

    bool correcta;
    const char* mensajeError = "No es la llave de este nivel.";

    if (MODO_DEMO_UID) {
        correcta = !uidYaUsado(uid);
        if (!correcta) mensajeError = "Esa tarjeta ya fue utilizada.";
    } else {
        correcta = uid.equalsIgnoreCase(UID_TARJETAS[i]);
    }

    lector.PICC_HaltA();

    if (!correcta) {
        muestraResultadoTarjeta(false, uid, mensajeError);
        delay(1200);
        dibujaBuscaTarjeta();
        return;
    }

    if (MODO_DEMO_UID) {
        uidDemo[i] = uid;
        memoria.putString((String("uid") + i).c_str(), uid);
    }

    muestraResultadoTarjeta(true, uid, "Has abierto este nivel.");
    delay(2500); // da tiempo a apuntar el UID mostrado en pantalla

    if (i == 2) cambiaEscena(VICTORIA);
    else cambiaEscena((Escena)((int)escena + 1));
}

// Mantener G0 inicia un juego nuevo, pero esperamos a que se suelte antes
// de reiniciar: arrancar con G0 pulsado meteria el ESP32 en el bootloader.
bool gestionaReinicio() {
    if (!esperandoSoltarG0 && M5Cardputer.BtnA.pressedFor(3000)) {
        esperandoSoltarG0 = true;
        limpiaPantalla(YELLOW, "JUEGO NUEVO");
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 45);
        M5Cardputer.Display.println("Suelta G0...");
    }

    if (!esperandoSoltarG0) return false;

    if (M5Cardputer.BtnA.wasReleased()) {
        apagaHex();
        memoria.clear();
        M5Cardputer.Display.setCursor(4, 75);
        M5Cardputer.Display.println("Reiniciando");
        delay(400);
        ESP.restart();
    }
    return true;
}
