/**
 * L4_RfidLeer — "¿Quien eres?"
 * ---------------------------------------------------------------
 * Placa:  M5Stack Cardputer ADV (ESP32-S3FN8)
 * Necesita: Unit RFID2 (chip WS1850S, compatible MFRC522, I2C 0x28)
 *           + tarjetas / llaveros de 13,56 MHz
 *
 * CONEXION
 *   RFID2 -> Port A (Grove).  Directo, o a traves del Unit Hub.
 *   En Port A el I2C es:  SDA = G2 (amarillo)   SCL = G1 (blanco)
 *
 * NOTA TECNICA (por que PIN_RESET_FALSO)
 *   La libreria MFRC522_I2C fue escrita para modulos que exponen un
 *   pin de RESET. El Unit RFID2 no lo saca por el Grove, asi que le
 *   damos un pin libre del conector EXT que no esta conectado a nada.
 *   La libreria lo pone en alto y sigue; el chip se resetea por
 *   software. Es un apaño legitimo y muy comun.
 *
 * QUE APRENDEMOS
 *   - Que cada tarjeta lleva un numero unico (UID) grabado de fabrica.
 *   - Que I2C es un "cable compartido" donde cada aparato tiene
 *     una direccion (la del RFID2 es 0x28).
 *
 * COMO SE USA
 *   Acerca una tarjeta. Sale su UID en pantalla y por el Serial.
 *   👉 APUNTA LOS UID: los necesitaremos para el escape room.
 * ---------------------------------------------------------------
 */

#include "M5Cardputer.h"
#include <Wire.h>
#include "MFRC522_I2C.h"

// ===== 🔧 CAMBIA ESTO =====================================
#define DIR_RFID2         0x28   // direccion I2C del Unit RFID2
#define PIN_RESET_FALSO      6   // G6: pin libre del conector EXT
#define PIN_SDA              2   // G2 = amarillo del Grove
#define PIN_SCL              1   // G1 = blanco  del Grove
// ==========================================================

MFRC522_I2C lector(DIR_RFID2, PIN_RESET_FALSO);

String ultimoUID = "";
int contador = 0;

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::Font0);
    M5Cardputer.Display.setTextSize(2);

    Serial.begin(115200);

    // Arrancamos el I2C del Port A con los pines del Cardputer ADV.
    Wire.begin(PIN_SDA, PIN_SCL, 100000);
    lector.PCD_Init();

    // Comprobamos que el lector responde de verdad.
    byte version = lector.PCD_ReadRegister(MFRC522_I2C::VersionReg);
    Serial.printf("Version del chip: 0x%02X\n", version);

    M5Cardputer.Display.fillScreen(BLACK);
    if (version == 0x00 || version == 0xFF) {
        M5Cardputer.Display.setTextColor(RED, BLACK);
        M5Cardputer.Display.setCursor(4, 4);
        M5Cardputer.Display.println("NO VEO EL RFID2");
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 40);
        M5Cardputer.Display.println("Revisa el cable");
        M5Cardputer.Display.println("del Port A");
    } else {
        M5Cardputer.Display.setTextColor(GREEN, BLACK);
        M5Cardputer.Display.setCursor(4, 4);
        M5Cardputer.Display.println("LECTOR LISTO");
        M5Cardputer.Display.setTextColor(WHITE, BLACK);
        M5Cardputer.Display.setCursor(4, 44);
        M5Cardputer.Display.println("Acerca una");
        M5Cardputer.Display.println("tarjeta...");
        M5Cardputer.Speaker.tone(1200, 80);
    }
}

// Convierte el UID (bytes sueltos) en un texto tipo "A3 2F 91 04"
String uidComoTexto() {
    String s = "";
    for (byte i = 0; i < lector.uid.size; i++) {
        if (lector.uid.uidByte[i] < 0x10) s += "0";
        s += String(lector.uid.uidByte[i], HEX);
        if (i < lector.uid.size - 1) s += " ";
    }
    s.toUpperCase();
    return s;
}

void loop() {
    M5Cardputer.update();

    // ¿Hay una tarjeta nueva delante? Si no, volvemos a empezar.
    if (!lector.PICC_IsNewCardPresent()) { delay(50); return; }
    if (!lector.PICC_ReadCardSerial())   { delay(50); return; }

    String uid = uidComoTexto();
    contador++;

    Serial.printf("Lectura #%d -> UID: %s\n", contador, uid.c_str());

    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(GREEN, BLACK);
    M5Cardputer.Display.setCursor(4, 4);
    M5Cardputer.Display.println("TARJETA!");
    M5Cardputer.Display.setTextColor(YELLOW, BLACK);
    M5Cardputer.Display.setCursor(4, 40);
    M5Cardputer.Display.println(uid);
    M5Cardputer.Display.setTextColor(DARKGREY, BLACK);
    M5Cardputer.Display.setCursor(4, 90);
    M5Cardputer.Display.printf("lecturas: %d", contador);

    M5Cardputer.Speaker.tone(1800, 60);

    ultimoUID = uid;
    delay(800);          // para no leer 20 veces la misma tarjeta
}
