/**
 * E3_EscapeRoomAvanzado — Las tres tarjetas de colores
 * ------------------------------------------------------------------
 * Placa: M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit HEX + Unit RFID2 + 3 tarjetas A073 preparadas con L5
 *
 * MECANICA
 *   1. Los jugadores encuentran una prueba de colores.
 *   2. La pantalla guia hasta tres tarjetas, una por una.
 *   3. Cada tarjeta hace aparecer una FIGURA COLOREADA en el HEX.
 *   4. Los jugadores encuentran un codificador de objetos a cifras.
 *   5. Deben deducir como relacionar tablero, colores, figuras y hojas.
 *   6. Las figuras ordenadas cuentan una historia que se explica al guardian.
 *   7. Las tres cifras forman el codigo del teclado.
 *
 * MONTAJE
 *   RFID2 -> Port A.
 *   HEX   -> G4 + GND + 5VOUT del conector EXT.
 *
 * Mantener G0 durante 3 segundos borra el progreso.
 * ------------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <Wire.h>
#include "MFRC522_I2C.h"

// ===== 🔧 CAMBIA ESTO: HISTORIA Y PISTAS =========================

const char* NOMBRE_JUEGO = "EL MENSAJE DE VEGA";

const char* INTRO_LINEAS[4] = {
    "Vega desaparecio.",
    "Dejo 3 tarjetas.",
    "Recupera fragmentos",
    "y descubre su mensaje."
};

const char* MENSAJE_FINAL[2] = {
    "Vega tenia razon.",
    "La puerta se abrio."
};

const char* RETO_HISTORIA[3] = {
    "3 figuras",
    "un mensaje.",
    "Como se leen?"
};

// Lugares de las dos hojas. Solo se imprimen por Serial para el organizador.
const char* NOMBRE_ESCONDITE_PRUEBA = "SALON, CERCA DE UNA LAMPARA";
const char* PISTA_PRUEBA[3] = {
    "Donde os reunis",
    "cerca de una luz",
    "contra oscuridad"
};

const char* NOMBRE_ESCONDITE_CODIFICADOR = "COCINA";
const char* PISTA_CODIFICADOR[3] = {
    "Una receta cambia",
    "cosas distintas",
    "en algo nuevo."
};

// Se muestran en este orden. Las tarjetas se aceptan una por una.
const char* NOMBRE_ESCONDITE[3] = { "LIBROS", "CDS", "JUGUETES" };

const char* PISTAS_BUSQUEDA[3][3] = {
    { "Muchas voces", "esperan de pie.", "En silencio." },
    { "Canciones quietas", "esperan girar.", "En silencio." },
    { "Juegos y munecos", "esperan juntos.", "Otra aventura." }
};

enum SimboloId : uint8_t {
    SIM_FLECHA,
    SIM_CRUZ,
    SIM_X,
    SIM_ROMBO,
    SIM_LINEA,
    SIM_ANILLO
};

struct TarjetaJuego {
    const char* textoTarjeta;  // Debe coincidir exactamente con L5.
    uint8_t r, g, b;
    uint16_t colorPantalla;
    SimboloId simbolo;
};

// Orden de ENCUENTRO: libros, CDs, juguetes.
// La pantalla no enseña el nombre de la figura: hay que reconocerla en el HEX.
const TarjetaJuego TARJETAS[3] = {
    { "E3|RO|FLECHA", 255,   0,   0, RED,   SIM_FLECHA },
    { "E3|AZ|ANILLO",   0,  70, 255, BLUE,  SIM_ANILLO },
    { "E3|VE|X",        0, 255,  40, GREEN, SIM_X      }
};

// Resultado de la prueba anterior: VERDE=1, ROJO=2, AZUL=3.
// Las figuras ordenadas forman X -> FLECHA -> ANILLO:
// "Desde la X, sigue la flecha hasta el portal". Sus cifras forman 9 4 8.
// Cambiar este array cambia automaticamente el codigo correcto.
const uint8_t ORDEN_COLORES[3] = { 2, 0, 1 };

// ===== 🔧 CAMBIA ESTO: HARDWARE =================================

#define PIN_HEX                4
#define NUM_LEDS              37
const int BRILLO_HEX          = 25;  // mantener <= 60 con bateria

#define DIR_RFID2           0x28
#define PIN_RESET_FALSO        6
#define PIN_SDA                2
#define PIN_SCL                1

// ==================================================================

// Las A073 son F08, organizadas como MIFARE Classic 1K.
// Solo leemos el bloque de datos 4. Nunca tocamos bloque 0 ni trailers.
const byte BLOQUE_CODIGO = 4;

struct SimboloHex {
    const char* nombre;
    char digitoTabla;
    const char* patron;
};

// Numeracion logica por filas del NeoHEX: 4 + 5 + 6 + 7 + 6 + 5 + 4.
// Cada patron tiene 37 caracteres: 1=LED encendido, 0=apagado.
// La tabla impresa incluye los seis; esta partida usa X, FLECHA y ANILLO.
const SimboloHex SIMBOLOS[6] = {
    { "FLECHA", '4',
      "0010" "00010" "000011" "1111111" "000011" "00010" "0010" },
    { "CRUZ", '2',
      "0110" "00100" "001100" "1111111" "001100" "00100" "0110" },
    { "X", '9',
      "1001" "10001" "010010" "0011100" "010010" "10001" "1001" },
    { "ROMBO", '7',
      "0110" "01010" "010010" "1000001" "010010" "01010" "0110" },
    { "LINEA", '5',
      "0000" "00000" "000000" "1111111" "000000" "00000" "0000" },
    { "ANILLO", '8',
      "0110" "10001" "100001" "1001001" "100001" "10001" "0110" }
};

enum Escena : uint8_t {
    INTRO,
    BUSCA_PRUEBA_ORDEN,
    BUSCA_1,
    REVELA_1,
    BUSCA_2,
    REVELA_2,
    BUSCA_3,
    REVELA_3,
    BUSCA_CODIFICADOR,
    DESCIFRA_HISTORIA,
    CLAVE_FINAL,
    VICTORIA
};

Adafruit_NeoPixel hex(NUM_LEDS, PIN_HEX, NEO_GRB + NEO_KHZ800);
MFRC522_I2C lector(DIR_RFID2, PIN_RESET_FALSO);
MFRC522_I2C::MIFARE_Key claveFabrica;
Preferences memoriaInterna;

Escena escena = INTRO;
bool rfidListo = false;
String codigoEscrito;
int intentosCodigo = 0;
uint16_t pasoVictoria = 0;
bool esperandoSoltarG0 = false;

void dibujaEscena();
void cambiaEscena(Escena nueva);
void procesaTeclado();
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

    for (byte i = 0; i < MFRC522_I2C::MF_KEY_SIZE; i++) {
        claveFabrica.keyByte[i] = 0xFF;
    }

    hex.begin();
    hex.setBrightness(min(BRILLO_HEX, 60));
    hex.clear();
    hex.show();

    Wire.begin(PIN_SDA, PIN_SCL, 100000);
    lector.PCD_Init();
    delay(80);
    byte version = lector.PCD_ReadRegister(MFRC522_I2C::VersionReg);
    rfidListo = version != 0x00 && version != 0xFF;
    Serial.printf("RFID2, version del chip: 0x%02X\n", version);

    // Namespace nuevo para no reutilizar el progreso del antiguo E3.
    memoriaInterna.begin("escape3deduce", false);
    byte guardada = memoriaInterna.getUChar("escena", INTRO);
    escena = guardada <= VICTORIA ? (Escena)guardada : INTRO;

    dibujaEscena();
}

void loop() {
    M5Cardputer.update();
    if (gestionaReinicio()) return;

    procesaTeclado();
    if (escena == BUSCA_1 || escena == BUSCA_2 || escena == BUSCA_3) {
        actualizaRfid();
    }

    if (escena == VICTORIA) {
        for (int i = 0; i < NUM_LEDS; i++) {
            uint16_t tono = pasoVictoria * 256 + i * (65536L / NUM_LEDS);
            hex.setPixelColor(i, hex.ColorHSV(tono));
        }
        hex.show();
        pasoVictoria++;
        delay(18);
    } else {
        delay(20);
    }
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

bool esRevelacion(Escena e) {
    return e == REVELA_1 || e == REVELA_2 || e == REVELA_3;
}

int indiceBusqueda() {
    if (escena == BUSCA_1 || escena == REVELA_1) return 0;
    if (escena == BUSCA_2 || escena == REVELA_2) return 1;
    return 2;
}

void pintaFigura(int indiceTarjeta) {
    const TarjetaJuego& t = TARJETAS[indiceTarjeta];
    const char* patron = SIMBOLOS[t.simbolo].patron;
    uint32_t color = hex.Color(t.r, t.g, t.b);
    hex.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
        if (patron[i] == '1') hex.setPixelColor(i, color);
    }
    hex.show();
}

void apagaHex() {
    hex.clear();
    hex.show();
}

void dibujaIntro() {
    limpiaPantalla(MAGENTA, NOMBRE_JUEGO);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 30);
    for (int i = 0; i < 4; i++) M5Cardputer.Display.println(INTRO_LINEAS[i]);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 98);
    M5Cardputer.Display.println("ENTER: comienza");
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 116);
    M5Cardputer.Display.println("G0: reinicia");
}

void dibujaBuscaPrueba() {
    limpiaPantalla(CYAN, "PRUEBA DE LUCES");
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    for (int i = 0; i < 3; i++) {
        M5Cardputer.Display.println(PISTA_PRUEBA[i]);
    }
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 112);
    M5Cardputer.Display.println("ENTER: continua");
    Serial.printf("Hoja de orden, escondite: %s\n", NOMBRE_ESCONDITE_PRUEBA);
}

void dibujaBusqueda() {
    if (!rfidListo) {
        limpiaPantalla(RED, "NO VEO EL RFID2");
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 42);
        M5Cardputer.Display.println("Revisa el Port A");
        M5Cardputer.Display.println("y reinicia.");
        return;
    }

    int i = indiceBusqueda();
    char cabecera[20];
    snprintf(cabecera, sizeof(cabecera), "RASTRO %d DE 3", i + 1);
    limpiaPantalla(MAGENTA, cabecera);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    for (int linea = 0; linea < 3; linea++) {
        M5Cardputer.Display.println(PISTAS_BUSQUEDA[i][linea]);
    }
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 106);
    M5Cardputer.Display.println("Acercala al RFID2");
    Serial.printf("Pista %d, escondite del organizador: %s\n",
                  i + 1, NOMBRE_ESCONDITE[i]);
}

void dibujaRevelacion() {
    int i = indiceBusqueda();
    const TarjetaJuego& t = TARJETAS[i];
    pintaFigura(i);

    char cabecera[18];
    snprintf(cabecera, sizeof(cabecera), "FRAGMENTO %d", i + 1);
    limpiaPantalla(t.colorPantalla, cabecera);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 31);
    M5Cardputer.Display.println("Observa color");
    M5Cardputer.Display.println("y figura del HEX.");
    M5Cardputer.Display.println("Busca su lugar");
    M5Cardputer.Display.println("en el tablero.");
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.println("Anota la figura.");
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 114);
    M5Cardputer.Display.println("ENTER: continua");
}

void dibujaBuscaCodificador() {
    limpiaPantalla(CYAN, "CODIFICADOR");
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 37);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    for (int i = 0; i < 3; i++) {
        M5Cardputer.Display.println(PISTA_CODIFICADOR[i]);
    }
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 113);
    M5Cardputer.Display.println("ENTER: continua");
    Serial.printf("Codificador, escondite: %s\n",
                  NOMBRE_ESCONDITE_CODIFICADOR);
}

void dibujaEntradaCodigo() {
    String visible = codigoEscrito;
    while (visible.length() < 3) visible += '_';

    // Los tres digitos son la accion principal de esta pantalla: se muestran
    // grandes para que el equipo pueda leerlos desde cierta distancia.
    M5Cardputer.Display.fillRect(0, 68, 240, 45, BLACK);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 78);
    M5Cardputer.Display.print("Codigo:");
    M5Cardputer.Display.setTextSize(3);
    M5Cardputer.Display.setCursor(94, 73);
    M5Cardputer.Display.print(visible);
}

void dibujaClave() {
    limpiaPantalla(CYAN, "ABRE EL PORTAL");
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 33);
    M5Cardputer.Display.println("Escribe la clave.");
    M5Cardputer.Display.println("DEL: borra");
    dibujaEntradaCodigo();
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 115);
    M5Cardputer.Display.println("ENTER: prueba");
}

void dibujaHistoria() {
    limpiaPantalla(MAGENTA, "MENSAJE OCULTO");
    M5Cardputer.Display.setTextSize(3);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 34);
    for (int i = 0; i < 3; i++) {
        M5Cardputer.Display.println(RETO_HISTORIA[i]);
    }
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 114);
    M5Cardputer.Display.println("ENTER: responde");
}

void dibujaVictoria() {
    limpiaPantalla(GREEN, "PORTAL ABIERTO");
    M5Cardputer.Display.setTextDatum(middle_center);
    M5Cardputer.Display.setTextFont(&fonts::FreeSansBold18pt7b);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.drawString("DESBLOQUEADO", 120, 54);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setCursor(4, 82);
    M5Cardputer.Display.println(MENSAJE_FINAL[0]);
    M5Cardputer.Display.println(MENSAJE_FINAL[1]);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 116);
    M5Cardputer.Display.println("G0: reinicia");

    M5Cardputer.Speaker.tone(650, 90);   delay(110);
    M5Cardputer.Speaker.tone(950, 90);   delay(110);
    M5Cardputer.Speaker.tone(1300, 110); delay(130);
    M5Cardputer.Speaker.tone(1750, 220);
}

void dibujaEscena() {
    switch (escena) {
        case INTRO:        dibujaIntro();       break;
        case BUSCA_PRUEBA_ORDEN: dibujaBuscaPrueba(); break;
        case BUSCA_1:
        case BUSCA_2:
        case BUSCA_3:      dibujaBusqueda();    break;
        case REVELA_1:
        case REVELA_2:
        case REVELA_3:     dibujaRevelacion();  break;
        case BUSCA_CODIFICADOR: dibujaBuscaCodificador(); break;
        case DESCIFRA_HISTORIA: dibujaHistoria(); break;
        case CLAVE_FINAL:  dibujaClave();       break;
        case VICTORIA:     dibujaVictoria();    break;
    }
}

void cambiaEscena(Escena nueva) {
    escena = nueva;
    memoriaInterna.putUChar("escena", (byte)escena);
    if (!esRevelacion(escena) && escena != VICTORIA) apagaHex();
    dibujaEscena();
}

// ------------------------------------------------------------------
// Lectura segura de las tarjetas A073 (F08 / Classic 1K).
// ------------------------------------------------------------------
bool esClassic(byte tipo) {
    return tipo == MFRC522_I2C::PICC_TYPE_MIFARE_MINI ||
           tipo == MFRC522_I2C::PICC_TYPE_MIFARE_1K ||
           tipo == MFRC522_I2C::PICC_TYPE_MIFARE_4K;
}

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

String textoDesdeBloque(const byte datos[16]) {
    String texto;
    for (int i = 0; i < 16; i++) {
        if (datos[i] == 0x00 || datos[i] == 0xFF) break;
        texto += (datos[i] >= 32 && datos[i] <= 126) ? (char)datos[i] : '?';
    }
    return texto;
}

bool leeCodigo(byte tipo, byte datos[16], byte& error) {
    memset(datos, 0, 16);
    if (!esClassic(tipo)) {
        error = MFRC522_I2C::STATUS_INVALID;
        lector.PICC_HaltA();
        return false;
    }

    error = lector.PCD_Authenticate(MFRC522_I2C::PICC_CMD_MF_AUTH_KEY_A,
                                   BLOQUE_CODIGO,
                                   &claveFabrica, &lector.uid);
    if (error != MFRC522_I2C::STATUS_OK) {
        lector.PICC_HaltA();
        return false;
    }

    byte buffer[18];
    byte tam = sizeof(buffer);
    error = lector.MIFARE_Read(BLOQUE_CODIGO, buffer, &tam);
    if (error == MFRC522_I2C::STATUS_OK) memcpy(datos, buffer, 16);

    lector.PICC_HaltA();
    lector.PCD_StopCrypto1();
    return error == MFRC522_I2C::STATUS_OK;
}

int buscaTarjeta(const String& texto) {
    for (int i = 0; i < 3; i++) {
        if (texto == TARJETAS[i].textoTarjeta) return i;
    }
    return -1;
}

void muestraErrorTarjeta(const char* titulo, const String& linea1,
                         const String& linea2) {
    limpiaPantalla(RED, titulo);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 42);
    M5Cardputer.Display.println(linea1);
    M5Cardputer.Display.println(linea2);
    M5Cardputer.Speaker.tone(170, 230);
    delay(1900);
    dibujaBusqueda();
}

void actualizaRfid() {
    if (!rfidListo || !lector.PICC_IsNewCardPresent()) return;
    if (!lector.PICC_ReadCardSerial()) return;

    byte tipo = lector.PICC_GetType(lector.uid.sak);
    String uid = uidComoTexto();
    byte datos[16];
    byte error = MFRC522_I2C::STATUS_OK;

    if (!leeCodigo(tipo, datos, error)) {
        Serial.printf("Fallo RFID %u, UID %s, tipo %u\n", error, uid.c_str(), tipo);
        muestraErrorTarjeta("NO PUEDO LEERLA",
                            "Tarjeta A073",
                            "sin preparar.");
        return;
    }

    String texto = textoDesdeBloque(datos);
    int encontrada = buscaTarjeta(texto);
    int esperada = indiceBusqueda();
    Serial.printf("UID %s -> '%s', encontrada %d, esperada %d\n",
                  uid.c_str(), texto.c_str(), encontrada, esperada);

    if (encontrada < 0) {
        muestraErrorTarjeta("TARJETA INVALIDA",
                            "No es tarjeta E3.",
                            "Prepara con L5.");
        return;
    }

    if (encontrada != esperada) {
        muestraErrorTarjeta("AUN NO LE TOCA",
                            "Es de otra pista.",
                            "Guardala despues.");
        return;
    }

    M5Cardputer.Speaker.tone(850, 70); delay(90);
    M5Cardputer.Speaker.tone(1400, 130);
    cambiaEscena((Escena)((int)escena + 1));
}

// ------------------------------------------------------------------
// Teclado y codigo final.
// ------------------------------------------------------------------
String codigoCorrecto() {
    String codigo;
    for (int i = 0; i < 3; i++) {
        SimboloId simbolo = TARJETAS[ORDEN_COLORES[i]].simbolo;
        codigo += SIMBOLOS[simbolo].digitoTabla;
    }
    return codigo;
}

void codigoFallido() {
    intentosCodigo++;
    M5Cardputer.Speaker.tone(160, 220);
    M5Cardputer.Display.fillRect(0, 103, 240, 31, BLACK);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(RED, BLACK);
    M5Cardputer.Display.setCursor(4, 106);
    if (intentosCodigo < 3) {
        M5Cardputer.Display.printf("Fallo: intento %d", intentosCodigo);
    } else {
        M5Cardputer.Display.println("Revisa pistas.");
    }
    delay(1500);
    codigoEscrito = "";
    dibujaClave();
}

void procesaTeclado() {
    if (!M5Cardputer.Keyboard.isChange() ||
        !M5Cardputer.Keyboard.isPressed()) return;

    auto teclas = M5Cardputer.Keyboard.keysState();

    if (escena == INTRO && teclas.enter) {
        cambiaEscena(BUSCA_PRUEBA_ORDEN);
        return;
    }

    if (escena == BUSCA_PRUEBA_ORDEN && teclas.enter) {
        cambiaEscena(BUSCA_1);
        return;
    }

    if (esRevelacion(escena) && teclas.enter) {
        if (escena == REVELA_3) cambiaEscena(BUSCA_CODIFICADOR);
        else cambiaEscena((Escena)((int)escena + 1));
        return;
    }

    if (escena == BUSCA_CODIFICADOR && teclas.enter) {
        cambiaEscena(DESCIFRA_HISTORIA);
        return;
    }

    if (escena == DESCIFRA_HISTORIA && teclas.enter) {
        cambiaEscena(CLAVE_FINAL);
        return;
    }

    if (escena != CLAVE_FINAL) return;

    for (auto c : teclas.word) {
        if (c >= '0' && c <= '9' && codigoEscrito.length() < 3) {
            codigoEscrito += c;
            M5Cardputer.Speaker.tone(430 + (c - '0') * 70, 25);
        }
    }

    if (teclas.del && codigoEscrito.length()) {
        codigoEscrito.remove(codigoEscrito.length() - 1);
        M5Cardputer.Speaker.tone(210, 25);
    }
    dibujaEntradaCodigo();

    if (teclas.enter) {
        if (codigoEscrito == codigoCorrecto()) cambiaEscena(VICTORIA);
        else codigoFallido();
    }
}

// Esperamos a soltar G0 antes de reiniciar para no entrar en el bootloader.
bool gestionaReinicio() {
    if (!esperandoSoltarG0 && M5Cardputer.BtnA.pressedFor(3000)) {
        esperandoSoltarG0 = true;
        limpiaPantalla(YELLOW, "PARTIDA NUEVA");
        M5Cardputer.Display.setTextSize(2);
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 50);
        M5Cardputer.Display.println("Suelta G0...");
    }

    if (!esperandoSoltarG0) return false;
    if (M5Cardputer.BtnA.wasReleased()) {
        apagaHex();
        memoriaInterna.clear();
        M5Cardputer.Display.setCursor(4, 78);
        M5Cardputer.Display.println("Reiniciando");
        delay(400);
        ESP.restart();
    }
    return true;
}
