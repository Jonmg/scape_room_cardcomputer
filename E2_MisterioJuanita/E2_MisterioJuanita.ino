/**
 * E2_MisterioJuanita — Las cuatro memorias
 * ------------------------------------------------------------------
 * Placa: M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit HEX + Unit RFID2 + 4 tarjetas RFID
 *
 * HISTORIA
 *   Juanita es un alma buena que intenta ayudar a su familia. Cuatro
 *   memorias se han perdido por la casa. Al reunirlas, el HEX revela
 *   el orden del codigo que permite resolver el misterio.
 *
 * MONTAJE ACTUAL
 *   RFID2 -> Port A.
 *   HEX   -> G4 + GND + 5VOUT del conector EXT.
 *
 * El juego guarda la escena actual. Mantener G0 durante 3 segundos
 * borra la partida y vuelve a empezar.
 * ------------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <Wire.h>
#include "MFRC522_I2C.h"
#include "audio_juanita_intro.h"

// ===== 🔧 CAMBIA ESTO =============================================

// false = montaje actual: RFID2 en Port A y HEX en G4 del EXT.
// true  = sin cable EXT: primero RFID2 y al final se cambia por el HEX.
const bool CAMBIO_MANUAL_MODULOS = false;

#define PIN_HEX             4   // usa 2 si el HEX vuelve al Port A
#define NUM_LEDS           37
const int BRILLO_HEX       = 25;

#define DIR_RFID2        0x28
#define PIN_RESET_FALSO     6
#define PIN_SDA              2
#define PIN_SCL              1

// En modo demo sirven cuatro tarjetas distintas cualesquiera.
const bool MODO_DEMO_UID = true;

// Despues de apuntar los UID, pegalos aqui y desactiva el modo demo.
const char* UID_TARJETAS[4] = {
    "A3 2F 91 04",
    "B4 00 12 34",
    "C5 00 56 78",
    "D6 00 90 12"
};

// La pantalla conduce directamente al dormitorio de cada tarjeta.
const char* PISTAS_RUTA[4][3] = {
    { "Busca la llave",    "donde los juguetes", "esperan compania." },
    { "Busca la llave",    "donde las historias", "duermen de pie."  },
    { "Busca la llave",    "junto a circulos",    "que guardan musica." },
    { "Busca ultima llave", "donde Juanita",      "pasaba buen rato." }
};

// ==================================================================

enum MemoriaId : uint8_t { MEM_ROJA, MEM_AZUL, MEM_VERDE, MEM_AMARILLA };

struct MemoriaFantasma {
    const char* nombre;
    char digito;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint16_t colorPantalla;
};

// Cada tarjeta revela una memoria. Los niños deben apuntarlas.
const MemoriaFantasma MEMORIAS[4] = {
    { "ROJA",     '4', 255,   0,   0, RED    },
    { "AZUL",     '8',   0,  70, 255, BLUE   },
    { "VERDE",    '2',   0, 255,   0, GREEN  },
    { "AMARILLA", '6', 255, 180,   0, YELLOW }
};

// Al final el HEX destella: VERDE, ROJO, AMARILLO, AZUL -> 2468.
const MemoriaId ORDEN_FINAL[4] = {
    MEM_VERDE, MEM_ROJA, MEM_AMARILLA, MEM_AZUL
};

enum Escena : uint8_t {
    INTRO,
    BUSCA_TARJETA_1,
    BUSCA_TARJETA_2,
    BUSCA_TARJETA_3,
    BUSCA_TARJETA_4,
    CAMBIO_A_HEX,
    CLAVE_FINAL,
    VICTORIA
};

Adafruit_NeoPixel hex(NUM_LEDS, PIN_HEX, NEO_GRB + NEO_KHZ800);
MFRC522_I2C lector(DIR_RFID2, PIN_RESET_FALSO);
Preferences memoriaInterna;

Escena escena = INTRO;
bool hexListo = false;
bool rfidListo = false;
byte versionRfid = 0;
String uidDemo[4];

String codigoEscrito;
int intentosCodigo = 0;
int destelloActual = -1;
bool destelloEncendido = false;
unsigned long proximoDestello = 0;
uint16_t pasoVictoria = 0;
bool esperandoSoltarG0 = false;

void iniciaHex();
bool iniciaRfid();
void dibujaEscena();
void cambiaEscena(Escena nueva);
void procesaTeclado();
void actualizaRfid();
void actualizaClaveHex();
bool gestionaReinicio();
void reproducePresentacionJuanita();

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Speaker.setVolume(70);
    Serial.begin(115200);

    memoriaInterna.begin("juanita", false);
    byte guardada = memoriaInterna.getUChar("escena", INTRO);
    escena = guardada <= VICTORIA ? (Escena)guardada : INTRO;

    for (int i = 0; i < 4; i++) {
        uidDemo[i] = memoriaInterna.getString((String("uid") + i).c_str(), "");
    }

    // Con el montaje simultaneo el HEX esta disponible desde el principio.
    // En modo manual solo se inicia despues de cambiar fisicamente el modulo.
    if (!CAMBIO_MANUAL_MODULOS || escena >= CAMBIO_A_HEX) iniciaHex();

    if (escena <= BUSCA_TARJETA_4) rfidListo = iniciaRfid();

    // Esta escena solo queda guardada mientras se hace el cambio fisico.
    if (escena == CAMBIO_A_HEX && hexListo) {
        cambiaEscena(CLAVE_FINAL);
        return;
    }

    dibujaEscena();
}

void loop() {
    M5Cardputer.update();
    if (gestionaReinicio()) return;

    procesaTeclado();

    if (escena >= BUSCA_TARJETA_1 && escena <= BUSCA_TARJETA_4) {
        actualizaRfid();
    }
    if (escena == CLAVE_FINAL) actualizaClaveHex();

    if (escena == VICTORIA && hexListo) {
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
// Hardware.
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
        pinMode(PIN_HEX, INPUT);
        hexListo = false;
    }
}

// La frase se reproduce desde memoria de programa, sin requerir SD ni red.
void reproducePresentacionJuanita() {
    M5Cardputer.Speaker.playWav(juanita_intro_wav, sizeof(juanita_intro_wav),
                                1, 0, true);
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
// Pantallas y escenas.
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
    limpiaPantalla(MAGENTA, "MISTERIO DE JUANITA");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 42);
    M5Cardputer.Display.println("Cuatro memorias se han perdido");
    M5Cardputer.Display.println("entre los susurros de la casa.");
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 88);
    M5Cardputer.Display.println("ENTER para escuchar");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 121);
    M5Cardputer.Display.println("G0 3 s: empezar una partida nueva");
}

void dibujaErrorRfid() {
    limpiaPantalla(RED, "NO VEO EL RFID2");
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 42);
    M5Cardputer.Display.println("Revisa Port A");
    M5Cardputer.Display.println("y reinicia.");
}

int indiceTarjetaActual() {
    return (int)escena - (int)BUSCA_TARJETA_1;
}

void dibujaRuta() {
    if (!rfidListo) {
        dibujaErrorRfid();
        return;
    }

    int i = indiceTarjetaActual();
    char titulo[25];
    snprintf(titulo, sizeof(titulo), "SUSURRO %d DE 4", i + 1);
    limpiaPantalla(MAGENTA, titulo);

    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    M5Cardputer.Display.println(PISTAS_RUTA[i][0]);
    M5Cardputer.Display.println(PISTAS_RUTA[i][1]);
    M5Cardputer.Display.println(PISTAS_RUTA[i][2]);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 112);
    M5Cardputer.Display.println("Trae la llave y acercala al RFID2");
}

void dibujaCambioHex() {
    limpiaPantalla(YELLOW, "CAMBIO AL HEX");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 31);
    M5Cardputer.Display.println("1. Desconecta USB y pon OFF");
    M5Cardputer.Display.println("2. Cambia RFID2 por HEX");
    M5Cardputer.Display.println("3. Port A en 5VOUT");
    M5Cardputer.Display.println("4. Pon ON y vuelve a encender");
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 109);
    M5Cardputer.Display.println("Las cuatro memorias estan guardadas.");
}

void dibujaEntradaCodigo() {
    String visible = codigoEscrito;
    while (visible.length() < 4) visible += '_';

    M5Cardputer.Display.fillRect(0, 79, 240, 27, BLACK);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 82);
    M5Cardputer.Display.print("Codigo: ");
    M5Cardputer.Display.println(visible);
}

void dibujaClaveFinal() {
    limpiaPantalla(CYAN, "MENSAJE DEL HEX");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 32);
    M5Cardputer.Display.println("El HEX ordena las cuatro memorias.");
    M5Cardputer.Display.println("Sustituye cada color por el numero");
    M5Cardputer.Display.println("que apuntaste al encontrarla.");
    dibujaEntradaCodigo();
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(4, 116);
    M5Cardputer.Display.println("ENTER comprueba   DEL borra");
}

void dibujaVictoria() {
    limpiaPantalla(GREEN, "MISTERIO RESUELTO");
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextFont(&fonts::FreeSansBold18pt7b);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.drawString("GRACIAS", 120, 58);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.drawString("El carino deja huellas", 120, 94);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 121);
    M5Cardputer.Display.println("G0 3 s: empezar otra vez");

    M5Cardputer.Speaker.tone(650, 100);  delay(120);
    M5Cardputer.Speaker.tone(900, 100);  delay(120);
    M5Cardputer.Speaker.tone(1250, 100); delay(120);
    M5Cardputer.Speaker.tone(1700, 200);
}

void dibujaEscena() {
    switch (escena) {
        case INTRO:             dibujaIntro();       break;
        case BUSCA_TARJETA_1:
        case BUSCA_TARJETA_2:
        case BUSCA_TARJETA_3:
        case BUSCA_TARJETA_4:   dibujaRuta();        break;
        case CAMBIO_A_HEX:      dibujaCambioHex();   break;
        case CLAVE_FINAL:       dibujaClaveFinal();  break;
        case VICTORIA:          dibujaVictoria();    break;
    }
}

void cambiaEscena(Escena nueva) {
    escena = nueva;
    memoriaInterna.putUChar("escena", (byte)escena);
    dibujaEscena();
}

// ------------------------------------------------------------------
// Tarjetas y memorias.
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
    for (int i = 0; i < 4; i++) {
        if (uidDemo[i].length() && uidDemo[i] == uid) return true;
    }
    return false;
}

void muestraTarjetaIncorrecta(const String& uid, const char* motivo) {
    limpiaPantalla(RED, "SUSURRO EQUIVOCADO");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 39);
    M5Cardputer.Display.println(motivo);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 71);
    M5Cardputer.Display.println(uid);
    M5Cardputer.Speaker.tone(170, 230);
}

void muestraMemoria(int i, const String& uid) {
    const MemoriaFantasma& m = MEMORIAS[i];
    limpiaPantalla(m.colorPantalla, "MEMORIA ENCONTRADA");

    M5Cardputer.Display.fillRoundRect(10, 40, 220, 40, 6, m.colorPantalla);
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextColor(i == MEM_AMARILLA ? BLACK : WHITE,
                                     m.colorPantalla);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.drawString(String(m.nombre) + " = " + m.digito,
                                   120, 60);

    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 98);
    M5Cardputer.Display.println("Apuntad color y numero.");
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 119);
    M5Cardputer.Display.println(uid);

    M5Cardputer.Speaker.tone(900, 80);  delay(100);
    M5Cardputer.Speaker.tone(1400, 140);
}

void terminaBusquedaRfid() {
    if (CAMBIO_MANUAL_MODULOS) {
        cambiaEscena(CAMBIO_A_HEX);
    } else {
        destelloActual = -1;
        destelloEncendido = false;
        proximoDestello = 0;
        cambiaEscena(CLAVE_FINAL);
    }
}

void actualizaRfid() {
    if (!rfidListo) return;
    if (!lector.PICC_IsNewCardPresent()) { delay(35); return; }
    if (!lector.PICC_ReadCardSerial())   { delay(35); return; }

    String uid = uidComoTexto();
    int i = indiceTarjetaActual();
    Serial.printf("Memoria %d -> UID: %s\n", i + 1, uid.c_str());

    bool correcta;
    const char* motivo = "No es la llave de este susurro.";
    if (MODO_DEMO_UID) {
        correcta = !uidYaUsado(uid);
        if (!correcta) motivo = "Esa llave ya revelo una memoria.";
    } else {
        correcta = uid.equalsIgnoreCase(UID_TARJETAS[i]);
    }

    lector.PICC_HaltA();

    if (!correcta) {
        muestraTarjetaIncorrecta(uid, motivo);
        delay(1600);
        dibujaRuta();
        return;
    }

    if (MODO_DEMO_UID) {
        uidDemo[i] = uid;
        memoriaInterna.putString((String("uid") + i).c_str(), uid);
    }

    muestraMemoria(i, uid);
    delay(4000);

    if (i == 3) terminaBusquedaRfid();
    else cambiaEscena((Escena)((int)escena + 1));
}

// ------------------------------------------------------------------
// Código final: el HEX ordena los números recogidos.
// ------------------------------------------------------------------
String codigoCorrecto() {
    String codigo;
    for (int i = 0; i < 4; i++) codigo += MEMORIAS[ORDEN_FINAL[i]].digito;
    return codigo;
}

void rellenaHex(MemoriaId cual) {
    const MemoriaFantasma& m = MEMORIAS[cual];
    uint32_t color = hex.Color(m.r, m.g, m.b);
    for (int i = 0; i < NUM_LEDS; i++) hex.setPixelColor(i, color);
    hex.show();
}

void actualizaClaveHex() {
    if (!hexListo || millis() < proximoDestello) return;

    if (destelloEncendido) {
        hex.clear();
        hex.show();
        destelloEncendido = false;
        proximoDestello = millis() + (destelloActual == 3 ? 1400 : 300);
    } else {
        destelloActual = (destelloActual + 1) % 4;
        rellenaHex(ORDEN_FINAL[destelloActual]);
        destelloEncendido = true;
        proximoDestello = millis() + 750;
    }
}

void codigoFallido() {
    intentosCodigo++;
    M5Cardputer.Speaker.tone(160, 220);
    M5Cardputer.Display.fillRect(0, 106, 240, 28, BLACK);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(RED, BLACK);
    M5Cardputer.Display.setCursor(4, 108);
    if (intentosCodigo < 3) {
        M5Cardputer.Display.printf("No coincide. Intento %d", intentosCodigo);
    } else {
        M5Cardputer.Display.println("Pista: ordena por colores del HEX.");
    }
    delay(1300);
    codigoEscrito = "";
    dibujaClaveFinal();
}

void codigoResuelto() {
    M5Cardputer.Speaker.tone(700, 80);   delay(100);
    M5Cardputer.Speaker.tone(1000, 80);  delay(100);
    M5Cardputer.Speaker.tone(1400, 100); delay(120);
    cambiaEscena(VICTORIA);
}

void procesaTeclado() {
    if (!M5Cardputer.Keyboard.isChange() ||
        !M5Cardputer.Keyboard.isPressed()) return;

    auto teclas = M5Cardputer.Keyboard.keysState();

    if (escena == INTRO && teclas.enter) {
        reproducePresentacionJuanita();
        cambiaEscena(BUSCA_TARJETA_1);
        return;
    }

    if (escena != CLAVE_FINAL) return;

    for (auto c : teclas.word) {
        if (c >= '0' && c <= '9' && codigoEscrito.length() < 4) {
            codigoEscrito += c;
            M5Cardputer.Speaker.tone(450 + (c - '0') * 75, 25);
        }
    }

    if (teclas.del && codigoEscrito.length() > 0) {
        codigoEscrito.remove(codigoEscrito.length() - 1);
        M5Cardputer.Speaker.tone(210, 25);
    }

    dibujaEntradaCodigo();

    if (teclas.enter) {
        if (codigoEscrito == codigoCorrecto()) codigoResuelto();
        else codigoFallido();
    }
}

// Esperamos a que se suelte G0 antes de reiniciar para no entrar en bootloader.
bool gestionaReinicio() {
    if (!esperandoSoltarG0 && M5Cardputer.BtnA.pressedFor(3000)) {
        esperandoSoltarG0 = true;
        limpiaPantalla(YELLOW, "PARTIDA NUEVA");
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 45);
        M5Cardputer.Display.println("Suelta G0...");
    }

    if (!esperandoSoltarG0) return false;

    if (M5Cardputer.BtnA.wasReleased()) {
        apagaHex();
        memoriaInterna.clear();
        M5Cardputer.Display.setCursor(4, 76);
        M5Cardputer.Display.println("Reiniciando");
        delay(400);
        ESP.restart();
    }
    return true;
}
