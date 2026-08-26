/**
 * L5_RfidEscribir — Prepara las tres tarjetas del escape room E3
 * ------------------------------------------------------------------
 * Placa: M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit RFID2 + tarjetas MIFARE Classic o Ultralight/NTAG
 *
 * CONEXION
 *   RFID2 -> Port A (SDA=G2, SCL=G1, direccion I2C 0x28).
 *
 * SEGURIDAD
 *   - Nunca escribe el UID, el bloque 0 ni los bloques de claves.
 *   - MIFARE Classic: usa los bloques de datos 4 y 5.
 *   - Ultralight/NTAG: usa las paginas de usuario 8 a 15.
 *   - Hay que elegir un texto y pulsar ENTER antes de acercar la tarjeta.
 *
 * Las tarjetas Classic deben conservar la clave de fabrica FF FF FF FF FF FF.
 * El programa escribe, vuelve a leer y solo confirma si los datos coinciden.
 * ------------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Wire.h>
#include "MFRC522_I2C.h"

// ===== 🔧 CAMBIA ESTO =============================================

#define DIR_RFID2           0x28
#define PIN_RESET_FALSO        6
#define PIN_SDA                2
#define PIN_SCL                1

// Cada texto debe tener como maximo 15 caracteres ASCII.
const char* TEXTOS_TARJETA[] = {
    "E3|RO|FLECHA",
    "E3|AZ|ANILLO",
    "E3|VE|X"
};

// ==================================================================

const byte BLOQUE_CODIGO_CLASSIC = 4;
const byte BLOQUE_SELLO_CLASSIC  = 5;
const byte PAGINA_CODIGO_UL      = 8;   // ocupa 8, 9, 10 y 11
const byte PAGINA_SELLO_UL       = 12;  // ocupa 12, 13, 14 y 15
const int NUM_TEXTOS = sizeof(TEXTOS_TARJETA) / sizeof(TEXTOS_TARJETA[0]);

enum Modo : uint8_t { MENU, ESPERA_LECTURA, ESPERA_ESCRITURA };

MFRC522_I2C lector(DIR_RFID2, PIN_RESET_FALSO);
MFRC522_I2C::MIFARE_Key claveFabrica;

Modo modo = MENU;
int textoElegido = -1;
bool rfidListo = false;

void dibujaMenu();
void procesaTeclado();
void procesaTarjeta();

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Speaker.setVolume(65);
    Serial.begin(115200);

    for (byte i = 0; i < MFRC522_I2C::MF_KEY_SIZE; i++) {
        claveFabrica.keyByte[i] = 0xFF;
    }

    Wire.begin(PIN_SDA, PIN_SCL, 100000);
    lector.PCD_Init();
    delay(80);
    byte version = lector.PCD_ReadRegister(MFRC522_I2C::VersionReg);
    rfidListo = version != 0x00 && version != 0xFF;
    Serial.printf("RFID2, version del chip: 0x%02X\n", version);

    dibujaMenu();
}

void loop() {
    M5Cardputer.update();
    procesaTeclado();
    if (modo != MENU) procesaTarjeta();
    delay(20);
}

// ------------------------------------------------------------------
// Pantalla y teclado.
// ------------------------------------------------------------------
void titulo(uint16_t color, const char* texto) {
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextDatum(top_left);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(color, BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.println(texto);
}

void dibujaMenu() {
    modo = MENU;
    titulo(CYAN, "ESCRITOR RFID E3");
    M5Cardputer.Display.setTextSize(1);

    if (!rfidListo) {
        M5Cardputer.Display.setTextColor(RED, BLACK);
        M5Cardputer.Display.setCursor(4, 38);
        M5Cardputer.Display.println("No veo el RFID2.");
        M5Cardputer.Display.println("Revisa el cable del Port A.");
        return;
    }

    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 29);
    M5Cardputer.Display.println("1-3: elige el texto de la tarjeta");
    M5Cardputer.Display.println("R: leer sin modificar");

    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 64);
    if (textoElegido >= 0) {
        M5Cardputer.Display.printf("Elegido: %s\n", TEXTOS_TARJETA[textoElegido]);
        M5Cardputer.Display.setTextColor(GREEN, BLACK);
        M5Cardputer.Display.println("ENTER arma la escritura");
    } else {
        M5Cardputer.Display.println("Todavia no hay texto elegido.");
    }

    M5Cardputer.Display.setTextColor(ORANGE, BLACK);
    M5Cardputer.Display.setCursor(4, 111);
    M5Cardputer.Display.println("La escritura reemplaza los datos E3.");
}

void dibujaEspera(bool escritura) {
    titulo(escritura ? ORANGE : CYAN,
           escritura ? "ESCRITURA ARMADA" : "MODO LECTURA");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 36);
    if (escritura) {
        M5Cardputer.Display.printf("Voy a guardar: %s\n", TEXTOS_TARJETA[textoElegido]);
        M5Cardputer.Display.println("y a borrar el sello anterior.");
    } else {
        M5Cardputer.Display.println("No se modificara ningun dato.");
    }
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 78);
    M5Cardputer.Display.println("Acerca UNA tarjeta al RFID2");
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 116);
    M5Cardputer.Display.println("DEL cancela");
}

void procesaTeclado() {
    if (!M5Cardputer.Keyboard.isChange() ||
        !M5Cardputer.Keyboard.isPressed()) return;

    auto teclas = M5Cardputer.Keyboard.keysState();

    if (teclas.del && modo != MENU) {
        dibujaMenu();
        return;
    }

    if (modo != MENU || !rfidListo) return;

    for (auto c : teclas.word) {
        if (c >= '1' && c < '1' + NUM_TEXTOS) {
            int i = c - '1';
            if (i < NUM_TEXTOS) {
                textoElegido = i;
                M5Cardputer.Speaker.tone(650 + i * 120, 35);
                dibujaMenu();
            }
        }
        if (c == 'r' || c == 'R') {
            modo = ESPERA_LECTURA;
            dibujaEspera(false);
        }
    }

    if (teclas.enter && textoElegido >= 0) {
        modo = ESPERA_ESCRITURA;
        M5Cardputer.Speaker.tone(950, 60);
        dibujaEspera(true);
    }
}

// ------------------------------------------------------------------
// Operaciones RFID. Solo zonas de datos, nunca UID ni claves.
// ------------------------------------------------------------------
bool esClassic(byte tipo) {
    return tipo == MFRC522_I2C::PICC_TYPE_MIFARE_MINI ||
           tipo == MFRC522_I2C::PICC_TYPE_MIFARE_1K ||
           tipo == MFRC522_I2C::PICC_TYPE_MIFARE_4K;
}

bool esUltralight(byte tipo) {
    return tipo == MFRC522_I2C::PICC_TYPE_MIFARE_UL;
}

const char* nombreTipo(byte tipo) {
    if (tipo == MFRC522_I2C::PICC_TYPE_MIFARE_MINI) return "MIFARE Mini";
    if (tipo == MFRC522_I2C::PICC_TYPE_MIFARE_1K)   return "MIFARE Classic 1K";
    if (tipo == MFRC522_I2C::PICC_TYPE_MIFARE_4K)   return "MIFARE Classic 4K";
    if (tipo == MFRC522_I2C::PICC_TYPE_MIFARE_UL)   return "Ultralight / NTAG";
    return "tipo no compatible";
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

void creaBloque(const char* texto, byte destino[16]) {
    memset(destino, 0, 16);
    strncpy((char*)destino, texto, 15);
}

void terminaSesion(bool usabaCifrado) {
    lector.PICC_HaltA();
    if (usabaCifrado) lector.PCD_StopCrypto1();
}

bool leeDosZonas(byte tipo, byte codigo[16], byte sello[16], byte& error) {
    byte buffer[18];
    byte tam;
    bool cifrado = esClassic(tipo);

    memset(codigo, 0, 16);
    memset(sello, 0, 16);

    if (cifrado) {
        error = lector.PCD_Authenticate(MFRC522_I2C::PICC_CMD_MF_AUTH_KEY_A,
                                       BLOQUE_CODIGO_CLASSIC,
                                       &claveFabrica, &lector.uid);
        if (error != MFRC522_I2C::STATUS_OK) {
            terminaSesion(false);
            return false;
        }

        tam = sizeof(buffer);
        error = lector.MIFARE_Read(BLOQUE_CODIGO_CLASSIC, buffer, &tam);
        if (error == MFRC522_I2C::STATUS_OK) memcpy(codigo, buffer, 16);
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(BLOQUE_SELLO_CLASSIC, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(sello, buffer, 16);
        }
    } else if (esUltralight(tipo)) {
        tam = sizeof(buffer);
        error = lector.MIFARE_Read(PAGINA_CODIGO_UL, buffer, &tam);
        if (error == MFRC522_I2C::STATUS_OK) memcpy(codigo, buffer, 16);
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(PAGINA_SELLO_UL, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(sello, buffer, 16);
        }
    } else {
        error = MFRC522_I2C::STATUS_INVALID;
        terminaSesion(false);
        return false;
    }

    terminaSesion(cifrado);
    return error == MFRC522_I2C::STATUS_OK;
}

bool escribeYVerifica(byte tipo, const char* texto, byte& error) {
    byte codigo[16];
    byte selloVacio[16] = {0};
    byte leidoCodigo[16] = {0};
    byte leidoSello[16] = {0};
    byte buffer[18];
    byte tam;
    bool cifrado = esClassic(tipo);

    creaBloque(texto, codigo);

    if (cifrado) {
        error = lector.PCD_Authenticate(MFRC522_I2C::PICC_CMD_MF_AUTH_KEY_A,
                                       BLOQUE_CODIGO_CLASSIC,
                                       &claveFabrica, &lector.uid);
        if (error == MFRC522_I2C::STATUS_OK) {
            error = lector.MIFARE_Write(BLOQUE_CODIGO_CLASSIC, codigo, 16);
        }
        if (error == MFRC522_I2C::STATUS_OK) {
            error = lector.MIFARE_Write(BLOQUE_SELLO_CLASSIC, selloVacio, 16);
        }
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(BLOQUE_CODIGO_CLASSIC, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(leidoCodigo, buffer, 16);
        }
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(BLOQUE_SELLO_CLASSIC, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(leidoSello, buffer, 16);
        }
    } else if (esUltralight(tipo)) {
        error = MFRC522_I2C::STATUS_OK;
        for (byte i = 0; i < 4 && error == MFRC522_I2C::STATUS_OK; i++) {
            error = lector.MIFARE_Ultralight_Write(PAGINA_CODIGO_UL + i,
                                                   codigo + i * 4, 4);
        }
        for (byte i = 0; i < 4 && error == MFRC522_I2C::STATUS_OK; i++) {
            error = lector.MIFARE_Ultralight_Write(PAGINA_SELLO_UL + i,
                                                   selloVacio + i * 4, 4);
        }
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(PAGINA_CODIGO_UL, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(leidoCodigo, buffer, 16);
        }
        if (error == MFRC522_I2C::STATUS_OK) {
            tam = sizeof(buffer);
            error = lector.MIFARE_Read(PAGINA_SELLO_UL, buffer, &tam);
            if (error == MFRC522_I2C::STATUS_OK) memcpy(leidoSello, buffer, 16);
        }
    } else {
        error = MFRC522_I2C::STATUS_INVALID;
        terminaSesion(false);
        return false;
    }

    terminaSesion(cifrado);
    return error == MFRC522_I2C::STATUS_OK &&
           memcmp(codigo, leidoCodigo, 16) == 0 &&
           memcmp(selloVacio, leidoSello, 16) == 0;
}

void muestraResultado(bool correcto, const char* cabecera,
                      const String& linea1, const String& linea2,
                      const String& uid, byte error = MFRC522_I2C::STATUS_OK) {
    titulo(correcto ? GREEN : RED, cabecera);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(WHITE, BLACK);
    M5Cardputer.Display.setCursor(4, 35);
    M5Cardputer.Display.println(linea1);
    M5Cardputer.Display.println(linea2);
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 83);
    M5Cardputer.Display.println(uid);
    if (!correcto) {
        M5Cardputer.Display.setTextColor(ORANGE, BLACK);
        M5Cardputer.Display.printf("Error RFID: %u", error);
        M5Cardputer.Speaker.tone(170, 250);
    } else {
        M5Cardputer.Speaker.tone(850, 70);  delay(90);
        M5Cardputer.Speaker.tone(1350, 120);
    }
}

void procesaTarjeta() {
    if (!rfidListo || !lector.PICC_IsNewCardPresent()) return;
    if (!lector.PICC_ReadCardSerial()) return;

    byte tipo = lector.PICC_GetType(lector.uid.sak);
    String uid = uidComoTexto();
    Serial.printf("UID %s, tipo %s\n", uid.c_str(), nombreTipo(tipo));

    if (!esClassic(tipo) && !esUltralight(tipo)) {
        terminaSesion(false);
        muestraResultado(false, "NO COMPATIBLE", nombreTipo(tipo),
                         "No se ha escrito nada.", uid,
                         MFRC522_I2C::STATUS_INVALID);
        delay(2200);
        dibujaMenu();
        return;
    }

    byte error = MFRC522_I2C::STATUS_OK;
    if (modo == ESPERA_LECTURA) {
        byte codigo[16], sello[16];
        bool ok = leeDosZonas(tipo, codigo, sello, error);
        String tCodigo = textoDesdeBloque(codigo);
        String tSello = textoDesdeBloque(sello);
        if (!tCodigo.length()) tCodigo = "(vacio)";
        if (!tSello.length()) tSello = "(sin sello)";
        muestraResultado(ok, ok ? "TARJETA LEIDA" : "FALLO DE LECTURA",
                         "Codigo: " + tCodigo, "Sello: " + tSello,
                         uid, error);
    } else if (modo == ESPERA_ESCRITURA) {
        bool ok = escribeYVerifica(tipo, TEXTOS_TARJETA[textoElegido], error);
        muestraResultado(ok,
                         ok ? "ESCRITURA CORRECTA" : "FALLO AL ESCRIBIR",
                         String(nombreTipo(tipo)),
                         ok ? String(TEXTOS_TARJETA[textoElegido])
                            : String("Nada confirmado; prueba otra vez."),
                         uid, error);
    }

    delay(2600);
    dibujaMenu();
}
