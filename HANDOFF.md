# Traspaso — Proyecto CardputerLab

Contexto para continuar el trabajo con otro asistente.
Fecha del traspaso: 2026-08-25. Usuario: jmartin (Linux, Ubuntu, bash).

---

## 1. Objetivo del proyecto

Cacharrear con una **M5Stack Cardputer ADV** y enseñar programación a
sobrinos de **9 a 14 años**. Dos fases:

1. **Ahora:** probar sensores/actuadores uno a uno con ejemplos sencillos.
2. **Después:** montar un **escape room** con los niños, donde leer una
   tarjeta RFID desencadena pistas en pantalla y en la tira de LEDs.

Prioridad del usuario: que sea **didáctico** y que los niños toquen código.

---

## 2. Hardware

| Pieza | Detalle |
|---|---|
| M5Stack **Cardputer ADV** | ESP32-S3FN8, módulo StampS3A |
| **Unit HEX** (NeoHEX) | 37 LEDs WS2812, bus digital de un hilo |
| **Unit RFID2** | chip WS1850S (compatible MFRC522), I2C dirección `0x28` |
| **Unit Hub** | repartidor 1→6, **pasivo** |
| Tarjetas RFID 13,56 MHz | varias |

### Pinout del Cardputer ADV (verificado leyendo M5Unified 0.2.13)

| Qué | Pines |
|---|---|
| **Port A (Grove)** | negro=GND, rojo=5V, **amarillo=G2**, **blanco=G1** |
| I2C externo (Port A) | SDA=**G2**, SCL=**G1** |
| I2C interno | SDA=G8, SCL=G9 (teclado TCA8418, IMU BMI270, códec ES8311) |
| microSD | CLK=G40, MOSI=G14, MISO=G39, CS=G12 |
| Altavoz I2S | BCK=G41, WS=G43, DOUT=G42 |
| Botón | G0 |
| IR | G44 |
| EXT 14 pines — GPIO libres | G3, G4, G5, G6, G13, G15 + 5V + GND |

> El Cardputer ADV **no lleva NFC integrado** (confirmado en docs.m5stack.com).
> Por eso hace falta el Unit RFID2.

> ⚠️ El pinout del **conector EXT** viene de la doc web de M5Stack y la tabla
> se renderizó mal al leerla. **Está sin verificar contra la placa real.**
> Confirmarlo antes de soldar o comprar cables.

---

## 3. Entorno de desarrollo (ya montado y funcionando)

- **arduino-cli 1.5.2** instalado en `~/bin/arduino-cli`.
  Coge automáticamente `~/.arduino15` (data) y `~/Arduino` (user), así que
  ve el core y las librerías que el usuario ya tenía. **Cero configuración.**
- `~/.profile` ya añade `~/bin` al PATH, pero la carpeta se creó en esta
  sesión → hace falta `source ~/.profile` o reiniciar sesión.
- **Core:** `m5stack:esp32` **2.1.4**. FQBN a usar:
  `m5stack:esp32:m5stack_cardputer`
  (no existe entrada "ADV" separada; **M5Unified detecta la variante en
  tiempo de ejecución** vía `board_M5CardputerADV`).
- **Arduino IDE 2.3.7** AppImage en `~/Descargas` — convive sin problema,
  comparte la misma carpeta de librerías.

### Librerías relevantes (ya instaladas, en `~/Arduino/libraries`)

| Librería | Versión | Nota |
|---|---|---|
| `M5Cardputer` | 1.1.1 | soporta ADV explícitamente (driver teclado TCA8418) |
| `M5Unified` | 0.2.13 | tiene `board_M5CardputerADV` y driver BMI270 |
| `M5GFX` | 0.2.19 | |
| `MFRC522_I2C` | 1.0 | de kkloesener; ver apaño del pin de reset (§5) |
| `Adafruit_NeoPixel` | 1.12.4 | usada para el HEX |
| `FastLED` | 3.9.13 | alternativa |

No existe librería dedicada al RFID2 en el índice de Arduino.

---

## 4. Qué se ha construido

Carpeta **`~/Arduino/CardputerLab/`**:

```
CardputerLab/
├── README.md
├── HANDOFF.md          <- este archivo
├── sube.sh             <- compila + sube + monitor serie
├── L1_Hola/            solo placa   — setup() vs loop(), pantalla
├── L2_Teclado/         solo placa   — teclado + altavoz
├── L3_HexArcoiris/     Unit HEX     — 4 animaciones, bucles for, RGB
├── L4_RfidLeer/        Unit RFID2   — lee UID, lo muestra por pantalla y Serial
├── L5_RfidEscribir/     RFID2 — escribe color/figura y verifica la lectura
├── E1_EscapeRoom/      HEX + RFID2 — escape room tutorial + guías
├── E2_MisterioJuanita/ HEX + RFID2 — cuatro tarjetas, código + guías
└── E3_EscapeRoomAvanzado/ HEX + RFID2 — tres tarjetas de colores + guías
```

**Estado:** las cuatro lecciones compilan. `L3_HexArcoiris` ya se ha probado
en hardware y funciona con el NeoHEX en Port A. `E1_EscapeRoom` compila
correctamente (37% de flash), pendiente de la primera prueba completa.

El usuario ya tiene el cableado simultáneo: **RFID2 en Port A** y **HEX en
G4 + GND + 5VOUT del EXT**. `E1_EscapeRoom` está configurado con
`CAMBIO_MANUAL_MODULOS = false` y `PIN_HEX = 4`.

`E2_MisterioJuanita` también usa ese montaje y compila correctamente (37%
de flash). Está en `MODO_DEMO_UID = true`: acepta cuatro tarjetas diferentes.
La pantalla conduce directamente a las cuatro tarjetas escondidas en los
cuatro dormitorios, sin notas de papel. El salón es la base; cocina y baños
son posibles lugares falsos. Cada RFID revela una memoria; el HEX las ordena
al final y el código de prueba es `2468`.

`L5_RfidEscribir` prepara tres tarjetas con `E3|RO|FLECHA`, `E3|AZ|ANILLO` y
`E3|VE|X`. Solo utiliza bloques/páginas de usuario, exige confirmación y
verifica lo escrito. Compila correctamente (35 %), pero no se ha cargado aún.

`E3_EscapeRoomAvanzado` es ahora la partida de las tres tarjetas de colores.
Primero hace buscar la prueba de luces; después busca las tarjetas en orden
libros → CDs → juguetes y finalmente hace encontrar el codificador en la
cocina. Cada tarjeta muestra en el HEX una figura coloreada. La pantalla no
explica cómo relacionar hojas, huecos, colores y figuras: los niños deben
deducirlo. La configuración de prueba es `VERDE=1`, `ROJO=2`, `AZUL=3`. Así se
forma X → flecha → anillo («desde la X, sigue la flecha hasta el portal») y la
clave final `948`. Las cifras no están escritas en las tarjetas y la partida no
escribe sobre ellas. Historia,
lugares, pistas, tarjetas, figuras y orden viven en el bloque configurable.
Todas sus guías y hojas para copiar están dentro de la carpeta E3, incluida
`PRUEBA_ORDEN_COLORES.md`, que permite deducir verde → rojo → azul.
E3 compila correctamente (37 %) y quedó cargado en `/dev/ttyACM0`; el arranque
real detectó el RFID2 con versión de chip `0x15`.

Convenciones usadas (mantenerlas si se añaden lecciones):
- Comentarios en español, tono para niños.
- Cabecera con placa, hardware necesario, conexión, y **origen** si está
  adaptado de un ejemplo oficial de M5Stack (con autor y licencia MIT).
- Un bloque `🔧 CAMBIA ESTO` al principio con los valores que los niños
  pueden tocar sin romper nada.

Uso: `./sube.sh L1_Hola` (o `--solo` para compilar sin placa).

---

## 5. Decisiones técnicas tomadas (y por qué)

### a) El conflicto HEX + RFID2 — importante

- El **RFID2 habla I2C** por G2 (SDA) / G1 (SCL).
- El **Unit HEX NO es I2C**: es bus digital WS2812 y usa G2 como línea de datos.
- **El Unit Hub NO resuelve esto**: es pasivo, sus 6 puertos comparten los
  mismos G1/G2. Las señales de píxeles rompen el bus I2C.
  El Hub sirve para **varios sensores I2C**, no para mezclar buses.

**Solución ya montada:** llevar el HEX al conector **EXT** a **G4 + 5VOUT +
GND**, dejando Port A solo para I2C. El usuario ya dispone del cable correcto
y los escape rooms usan ambos módulos simultáneamente.

### b) Escape room: UN solo firmware con escenas

El usuario preguntó si al leer una tarjeta se podría "cargar otro programa".
Se le recomendó **no** hacerlo: reflashear son ~40 s con portátil conectado
y mata el juego. En su lugar, **máquina de estados** en un único firmware:

```cpp
enum Escena { ESPERANDO, PISTA_1, PISTA_2, FINAL };
struct Tarjeta { const char* uid; Escena escena; const char* nombre; };
Tarjeta tarjetas[] = {
  {"A3 2F 91 04", PISTA_1, "Llave de bronce"},
  ...
};
```

Lees UID → buscas en la tabla → saltas de escena. Añadir una pista = añadir
una línea. **Evolución prevista para los mayores (13-14):** mover la tabla a
la microSD, para que puedan diseñar pistas sin recompilar (lección de
"datos vs. código").

### c) Consumo de los LEDs

La documentación específica del NeoHEX mide unos **568 mA** con los 37 LEDs
a blanco pleno. La estimación anterior de 2 A era genérica y demasiado alta
para estos WS2812C-2020. `L3_HexArcoiris` mantiene el tope de brillo en 60 y
el escape room arranca en 25 para cuidar la batería.

### d) Apaño del pin de reset en el RFID2

`MFRC522_I2C` exige un pin de RESET en el constructor, pero el Unit RFID2 no
lo saca por el Grove. Se le pasa **G6** (pin libre del EXT, sin conectar);
`PCD_Init()` lo pone en alto y continúa, el chip se resetea por software.
`L4_RfidLeer` imprime la versión del chip al arrancar: si sale `0x00` o `0xFF`
**no hay comunicación** y hay que revisar cableado/dirección.

### e) Memoria RFID reservada para E3

El escritor y E3 comparten este formato, sin tocar UID ni claves:

- MIFARE Classic: bloque 4 para el identificador E3 y bloque 5 reservado,
  que el escritor deja vacío.
- Ultralight/NTAG: páginas 8–11 para el identificador y 12–15 reservadas,
  que el escritor deja vacías.
- Cada zona ocupa 16 bytes y los textos configurables se limitan a 15
  caracteres ASCII más el terminador.
- En Classic se usa la clave de fábrica `FF FF FF FF FF FF`. Una tarjeta con
  otra clave o protección se rechaza.

---

## 6. Estado del USB y alimentación — bloqueo resuelto

El cable anterior era el problema. Con el cable de datos correcto la placa
enumera como **`/dev/ttyACM0`** y el usuario ya está compilando, subiendo y
probando sketches en hardware.

Detalles aprendidos en la prueba:

- El selector `5VIN`/`5VOUT` del Port A elige la alimentación del hilo rojo.
  Para trabajar desde batería se usará `5VOUT`.
- El interruptor lateral `ON/OFF` controla la batería. Con USB conectado la
  placa continúa encendida incluso en `OFF`; para cargar debe estar en `ON`.
- `G0` es botón programable y botón de entrada al bootloader durante arranque.

---

## 7. Siguiente paso concreto

1. Subir `L5_RfidEscribir` cuando se vayan a preparar las tarjetas.
2. Escribir y verificar una tarjeta roja/flecha, una azul/anillo y una
   verde/X siguiendo `L5_RfidEscribir/GUIA.md`.
3. Volver a subir E3, porque cada carga sustituye el programa anterior.
4. Reiniciar la partida manteniendo G0 tres segundos y comprobar en el HEX la
   flecha roja, el anillo azul y la X verde.
5. Ensayar la búsqueda de las dos hojas, la historia y la solución `948`, y
   montar la partida siguiendo `E3_EscapeRoomAvanzado/GUIA_PREPARACION.md`.

---

## 8. Fuentes consultadas

- https://docs.m5stack.com/en/core/Cardputer-Adv
- https://docs.m5stack.com/en/unit/rfid2
- https://docs.m5stack.com/en/arduino/projects/unit/unit_rfid
- https://www.nxp.com/products/rfid-nfc/mifare-hf/mifare-classic/mifare-classic-ev1-1k-4k%3AMF1S50YYX_V1
- https://docs.m5stack.com/en/homeassistant/light/unit_neo_hex
- https://github.com/m5stack/M5Stack/tree/master/examples/Unit/HEX_SK6812
- https://shop.m5stack.com/products/neo-hex-37-rgb-led-board-ws2812
