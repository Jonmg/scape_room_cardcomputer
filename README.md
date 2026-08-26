# CardputerLab

Ejemplos para **M5Stack Cardputer ADV** (ESP32-S3FN8) pensados para
cacharrear con peques de 9 a 14 años.

Todo vive aquí, incluso lo adaptado de los ejemplos oficiales de M5Stack
(cada sketch dice en su cabecera de dónde viene).

## Cómo subir un programa

```bash
./sube.sh L1_Hola            # compila + sube + monitor serie
./sube.sh L3_HexArcoiris --solo   # solo compila, sin placa conectada
./sube.sh E1_EscapeRoom      # primer juego completo
```

O desde el Arduino IDE: `Archivo > Abrir` la carpeta del sketch.
Placa: **M5Cardputer** (`m5stack:esp32:m5stack_cardputer`).

En cada sketch hay un bloque marcado `🔧 CAMBIA ESTO` con los valores
que se pueden tocar sin miedo. Ahí es donde empiezan los niños.

## Lecciones

| # | Sketch | Hardware | Qué se aprende |
|---|--------|----------|----------------|
| 1 | `L1_Hola` | solo placa | `setup()` vs `loop()`, coordenadas de pantalla |
| 2 | `L2_Teclado` | solo placa | entrada → proceso → salida |
| 3 | `L3_HexArcoiris` | Unit HEX | bucles `for`, colores RGB, consumo eléctrico |
| 4 | `L4_RfidLeer` | Unit RFID2 | I2C, identificadores únicos (UID) |
| 5 | `L5_RfidEscribir` | Unit RFID2 | memoria de usuario, lectura, escritura y verificación |

## Escape room 1: El laboratorio de las luces

`E1_EscapeRoom` ya contiene un recorrido completo:

1. El **HEX** emite cuatro colores. La pantalla da una tabla de
   sustitución color → número.
2. Los niños escriben el código resultante con el **teclado**.
3. El progreso se guarda. Se apaga la placa y se cambia HEX por RFID2.
4. El juego continúa buscando **tres tarjetas RFID diferentes**.
5. La tercera tarjeta abre la escena de victoria.

Guías de la partida tutorial:

- [`GUIA_PREPARACION.md`](E1_EscapeRoom/GUIA_PREPARACION.md): montaje, escondites,
  solución, ensayo y resolución de problemas.
- [`GUIA_NINOS.md`](E1_EscapeRoom/GUIA_NINOS.md): hoja imprimible sin soluciones para
  seguir la misión y anotar los resultados.

Ahora está configurado para mantener ambos módulos conectados:

```cpp
const bool CAMBIO_MANUAL_MODULOS = false;
#define PIN_HEX 4
const bool MODO_DEMO_UID = true;
```

El RFID2 va al Port A y el HEX a `G4 + GND + 5VOUT` del conector EXT.

El modo demo acepta tres tarjetas distintas y muestra sus UID. Cuando estén
anotados, se pegan en `UID_TARJETAS` y se cambia `MODO_DEMO_UID` a `false`.

La alternativa sin cable EXT es cambiar `PIN_HEX` a `2` y
`CAMBIO_MANUAL_MODULOS` a `true`; entonces hay que intercambiar HEX y RFID2
en Port A siguiendo las instrucciones que aparecen en pantalla.

## Escape room 2: El misterio de Juanita

`E2_MisterioJuanita` es una aventura más larga con ambiente de fantasmas,
cuatro tarjetas y una ruta por ocho estancias. Juanita es una presencia
protectora: los jugadores recuperan cuatro memorias de color y número, y el
HEX indica al final cómo ordenarlas para formar el código del teclado.

```bash
./sube.sh E2_MisterioJuanita
```

Material de la partida:

- [`GUIA_PREPARACION_E2.md`](E2_MisterioJuanita/GUIA_PREPARACION_E2.md): montaje, ruta,
  solución y dirección del juego.
- [`GUIA_NINOS_E2.md`](E2_MisterioJuanita/GUIA_NINOS_E2.md): hoja del equipo sin spoilers.

No utiliza notas escondidas: la pantalla conduce directamente a las cuatro
tarjetas de los dormitorios.

El escape room de prueba `E1_EscapeRoom` permanece sin cambios.

## Escritura RFID y escape room avanzado

`L5_RfidEscribir` prepara tres tarjetas con un identificador de color y figura:
`E3|RO|FLECHA`, `E3|AZ|ANILLO` y `E3|VE|X`. Trabaja en zonas de usuario,
no cambia el UID ni las claves y verifica los datos volviéndolos a leer. Consulta su
[`GUIA.md`](L5_RfidEscribir/GUIA.md) antes de usar tarjetas que contengan
información.

`E3_EscapeRoomAvanzado` guía la búsqueda de dos hojas y tres tarjetas. Cada
tarjeta hace que el HEX muestre una figura coloreada, pero la pantalla no
explica cómo relacionarla con los huecos, la prueba de las luces y el
codificador. Los jugadores deben descubrirlo. Las figuras cuentan «desde la X,
sigue la flecha hasta el portal» y después se traducen con la tabla. La
configuración de prueba usa `VERDE=1`, `ROJO=2`, `AZUL=3` y da `948`.

```bash
./sube.sh L5_RfidEscribir
./sube.sh E3_EscapeRoomAvanzado
```

Todo el material de E3 está en
[`E3_EscapeRoomAvanzado/`](E3_EscapeRoomAvanzado/README.md), incluidas las
guías, la tabla codificadora del HEX, el tablero imprimible y las soluciones
del organizador.

## Pinout del Cardputer ADV que usamos

| Qué | Pines |
|-----|-------|
| **Port A (Grove)** | negro=GND, rojo=5V, **amarillo=G2**, **blanco=G1** |
| I2C externo (Port A) | SDA = **G2**, SCL = **G1** |
| I2C interno | SDA = G8, SCL = G9 (teclado TCA8418, IMU BMI270, códec ES8311) |
| microSD | CLK=G40, MOSI=G14, MISO=G39, CS=G12 |
| Altavoz I2S | BCK=G41, WS=G43, DOUT=G42 |
| Botón G0 | G0 |
| IR | G44 |
| EXT 14 pines, libres | G3, G4, G5, G6, G13, G15 + 5V y GND |

## ⚠️ El conflicto HEX + RFID2 (y cómo se resuelve)

- El **RFID2 habla I2C** por G2 (SDA) y G1 (SCL).
- El **Unit HEX no es I2C**: es un bus digital WS2812 que usa G2 como
  línea de datos.

Si los enchufas a la vez al **Unit Hub**, no funciona: el Hub es un
repartidor **pasivo**, todos sus puertos comparten los mismos G1/G2.
Las señales de los LEDs se meten en el bus I2C y lo rompen.
El Hub es estupendo para **varios sensores I2C**, no para mezclar.

**Solución para usar los dos a la vez:** llevar el HEX al conector
**EXT**, a un GPIO libre (por ejemplo **G4**) + **5VOUT** + **GND**, y dejar
Port A / Hub solo para I2C. Luego basta cambiar en `L3_HexArcoiris`:

```cpp
#define PIN_HEX  4     // antes 2
```

Hace falta un cable Grove-a-jumper (hembra 2.54) para pinchar en el EXT.

## Consumo

La documentación del NeoHEX mide unos **568 mA** con los 37 LEDs a blanco
pleno. Los sketches usan brillo bajo y un tope de 60 para cuidar la batería
y evitar picos innecesarios.

## Alimentación del Port A en el Cardputer ADV

El selector junto al Port A decide de dónde sale el cable rojo del Grove:

- `5VOUT`: el Cardputer alimenta el módulo; es la posición para trabajar
  desde la batería.
- `5VIN`: usa la línea de entrada de 5 V; con USB conectado también tiene
  tensión.

El interruptor lateral `ON/OFF` controla la batería, no corta el USB. Para
cargar la batería debe estar en `ON`. El pulsador `G0` es programable y
también permite entrar en el bootloader al encender.
