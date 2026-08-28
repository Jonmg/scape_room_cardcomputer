# CardputerLab

Repositorio en español para aprender a programar la **M5Stack Cardputer ADV**
con Arduino, probar sus periféricos y construir pequeños *escape rooms* para
cacharrear con niños de 9 a 14 años.

La idea es avanzar de lo sencillo a lo completo: primero pantalla, teclado,
sonido, luces y RFID por separado; después se combinan en juegos con pistas,
tarjetas escondidas, códigos y escenas. Los ejemplos están muy comentados y
cada sketch tiene un bloque `🔧 CAMBIA ESTO` con valores que se pueden modificar
sin tener que entender todo el programa.

> **Aviso para jugadores:** las guías de preparación y parte de este README
> contienen soluciones. Si vas a jugar, deja que las lea únicamente quien
> organice la partida.

## Qué hay en el repositorio

```text
CardputerLab/
├── L1_Hola/                    pantalla, texto y batería
├── L2_Teclado/                 teclado, pantalla y altavoz
├── L3_HexArcoiris/             animaciones con 37 LED RGB
├── L4_RfidLeer/                lectura de tarjetas RFID
├── L5_RfidEscribir/            escritura segura de datos para E3
├── E1_EscapeRoom/              escape room de iniciación
├── E2_MisterioJuanita/         aventura de cuatro tarjetas
├── E3_EscapeRoomAvanzado/      juego completo con material imprimible
├── sube.sh                     compila, carga y abre el monitor serie
├── LICENSE                     licencia MIT
└── README.md                   esta guía
```

Las carpetas `L*` son lecciones independientes. Las carpetas `E*` contienen
un juego, su firmware y las guías necesarias para prepararlo. Cada sketch de
Arduino está dentro de una carpeta con su mismo nombre, como espera el IDE.

## La placa: M5Stack Cardputer ADV

El Cardputer ADV es un pequeño ordenador programable basado en un
**ESP32-S3FN8**. No ejecuta un sistema operativo de escritorio: al cargar un
sketch, ese programa controla directamente la pantalla, el teclado y el resto
del hardware.

| Elemento integrado | Para qué sirve en estos proyectos |
|---|---|
| ESP32-S3 de doble núcleo, hasta 240 MHz y 8 MB de flash | ejecuta los sketches y guarda el programa |
| Pantalla de 1,14", 240 × 135 píxeles | muestra menús, pistas, códigos y dibujos |
| Teclado de 56 teclas | introduce texto y soluciones |
| Altavoz de 1 W | reproduce tonos y confirma acciones |
| Batería interna de 1750 mAh | permite jugar sin mantener el USB conectado |
| Botón `G0` | entrada adicional y reinicio de las partidas |
| Wi-Fi y Bluetooth LE | disponibles para futuras ampliaciones |
| IMU BMI270 de seis ejes | mide movimiento y orientación; aún no se usa aquí |
| Emisor infrarrojo | puede controlar aparatos o formar parte de nuevas pruebas |
| Ranura microSD | permite guardar pistas y datos sin recompilar; ampliación futura |
| Port A Grove y conector EXT | conectan sensores y actuadores externos |

Documentación del fabricante:
[Cardputer ADV](https://docs.m5stack.com/en/core/Cardputer-Adv).

### Dos ideas básicas de Arduino

- `setup()` se ejecuta una sola vez al encender o reiniciar la placa. Se usa
  para preparar la pantalla, el teclado y los módulos.
- `loop()` se repite continuamente. Lee entradas, actualiza el estado del
  juego y produce salidas.

Una forma útil de pensar en cada programa es:

```text
entrada (teclado o RFID) → proceso (reglas del juego) → salida (pantalla, sonido o luces)
```

## Sensores y actuadores utilizados

### Unit HEX: luz como salida

El **Unit HEX** es un actuador formado por 37 LED RGB direccionables. Cada LED
puede recibir un color distinto, expresado normalmente con tres valores:
rojo, verde y azul, de 0 a 255. Todos comparten alimentación y una única línea
digital de datos; el programa los controla con `Adafruit_NeoPixel`.

Se utiliza para enseñar colores RGB, bucles `for`, animaciones y consumo
eléctrico. El brillo se mantiene bajo porque encender muchos LED a la vez
consume bastante más que dibujar en la pantalla.

Documentación: [M5Stack Unit HEX](https://docs.m5stack.com/en/unit/hex).

### Unit RFID2: tarjetas como entrada y memoria

El **Unit RFID2** es un lector/escritor de tarjetas de 13,56 MHz basado en el
chip WS1850S. Se comunica por el bus **I²C** y utiliza la dirección `0x28`.
El Cardputer ADV no incorpora NFC/RFID: todas las actividades con tarjetas de
este repositorio requieren este módulo externo.

- El **UID** identifica la tarjeta y viene grabado de fábrica.
- Algunas tarjetas también tienen memoria de usuario que se puede leer y
  escribir.
- `L4_RfidLeer` solo lee el UID.
- `L5_RfidEscribir` escribe zonas de usuario, nunca el UID, el bloque de
  fabricante ni los bloques de claves.

I²C permite conectar varios dispositivos a los mismos dos cables, `SDA` y
`SCL`, siempre que tengan direcciones compatibles. En el Cardputer ADV esos
cables salen por el Port A como `G2` y `G1`.

Documentación: [M5Stack Unit RFID2](https://docs.m5stack.com/en/unit/rfid2).

### Unit Hub: un repartidor, no un traductor

El **Unit Hub** es pasivo: duplica alimentación y señales, pero no crea buses
independientes ni cambia el protocolo. Es útil para varios módulos I²C, pero
no resuelve por sí solo el conflicto entre el RFID2 y el HEX explicado más
abajo.

## Ruta de aprendizaje

| Paso | Sketch | Hardware | Qué se practica |
|---|---|---|---|
| 1 | [`L1_Hola`](L1_Hola/L1_Hola.ino) | solo la placa | `setup()`, `loop()`, texto, coordenadas y batería |
| 2 | [`L2_Teclado`](L2_Teclado/L2_Teclado.ino) | solo la placa | entrada → proceso → salida, teclado y tonos |
| 3 | [`L3_HexArcoiris`](L3_HexArcoiris/L3_HexArcoiris.ino) | Unit HEX | bucles, píxeles, RGB, animación y brillo |
| 4 | [`L4_RfidLeer`](L4_RfidLeer/L4_RfidLeer.ino) | Unit RFID2 y tarjetas | I²C, dirección del módulo y UID |
| 5 | [`L5_RfidEscribir`](L5_RfidEscribir/L5_RfidEscribir.ino) | Unit RFID2 y tarjetas compatibles | memoria, lectura, escritura y verificación |

Una buena dinámica con niños es ejecutar primero el ejemplo sin tocarlo,
cambiar después una sola cosa del bloque `🔧 CAMBIA ESTO` y observar el
resultado. Color, nombre, volumen, brillo o velocidad dan cambios visibles sin
introducir demasiados conceptos a la vez.

## Escape rooms incluidos

| Juego | Nivel | Material | Mecánica principal |
|---|---|---|---|
| [`E1_EscapeRoom`](E1_EscapeRoom/) | iniciación | HEX, RFID2 y 3 tarjetas | descifrar colores, introducir un código y encontrar tarjetas |
| [`E2_MisterioJuanita`](E2_MisterioJuanita/) | intermedio | HEX, RFID2 y 4 tarjetas | recuperar cuatro memorias y ordenarlas con una señal luminosa |
| [`E3_EscapeRoomAvanzado`](E3_EscapeRoomAvanzado/) | avanzado | HEX, RFID2, 3 tarjetas preparadas y hojas impresas | relacionar colores, símbolos, pistas y un codificador |

### E1: El laboratorio de las luces

Es la partida tutorial. El HEX muestra una secuencia de colores, los jugadores
la convierten en un código y después siguen pistas para encontrar tres tarjetas
RFID. Incluye una
[guía de preparación](E1_EscapeRoom/GUIA_PREPARACION.md) y una
[hoja para los niños](E1_EscapeRoom/GUIA_NINOS.md).

### E2: El misterio de Juanita

Una aventura más larga con cuatro tarjetas escondidas. Cada una recupera una
memoria de color y número; al final, el HEX revela el orden correcto. Incluye
[guía de preparación](E2_MisterioJuanita/GUIA_PREPARACION_E2.md) y
[hoja del equipo](E2_MisterioJuanita/GUIA_NINOS_E2.md).

### E3: Las tres tarjetas de colores

La partida más abierta: los jugadores encuentran dos hojas y tres tarjetas.
Cada tarjeta genera una figura coloreada en el HEX, pero deben deducir cómo
relacionar las figuras, los colores, el tablero y el codificador. Todo el
material está explicado en el
[README específico de E3](E3_EscapeRoomAvanzado/README.md).

Antes de jugar, `L5_RfidEscribir` prepara las tres tarjetas con identificadores
de color y figura. Consulta su [guía de uso](L5_RfidEscribir/GUIA.md) y no uses
tarjetas que ya contengan información importante.

### Una partida, un firmware

Cada *escape room* mantiene sus escenas dentro de un único sketch. Al leer una
tarjeta RFID, el juego cambia de escena y muestra la siguiente pista: no carga
otro programa ni necesita un portátil durante la partida. Esto permite que el
equipo juegue con batería y que el progreso se guarde de forma local.

### Diseño de pantallas de juego

La pantalla del Cardputer es pequeña, pero el equipo debe poder leerla sin
acercarse a ella. En los *escape rooms*, toda la información necesaria para
avanzar —historia, pistas, estados, errores, resultados y acciones— se muestra
con `TextSize(2)` como mínimo. `TextSize(1)` se reserva para el pie de pantalla:
reinicio con G0, UID, modo demo o información técnica no esencial.

Con `fonts::Font0` a tamaño 2 caben aproximadamente 19 caracteres por línea
con los márgenes habituales. Cuando una frase no entra, se reescribe en líneas
cortas o se reparte en pantallas; no se hace la letra más pequeña. Los códigos
pueden usar tamaño 3 si siguen dejando protagonismo a la historia y a la acción
que el equipo debe realizar.

El altavoz se usa normalmente para tonos de confirmación, error o celebración.
También puede reproducir WAV integrado en el firmware o desde microSD. La voz
sintetizada se reserva para efectos robóticos intencionados: para narración es
preferible una grabación revisada por quien organiza la partida.

## Hardware necesario

Para las lecciones básicas basta el Cardputer ADV y un cable USB-C de datos.
Para completar todo el recorrido se utiliza:

- M5Stack Cardputer ADV.
- Unit HEX y cable Grove.
- Unit RFID2 y tarjetas o llaveros RFID compatibles de 13,56 MHz.
- Un cable Grove a conectores hembra de 2,54 mm para usar el HEX desde `G4`
  mientras el RFID2 permanece en el Port A.
- Opcionalmente, un Unit Hub para ampliar módulos I²C; no es necesario para
  los juegos actuales.

## Preparar el entorno

Los ejemplos se han probado con Arduino y estas librerías:

- `M5Cardputer`
- `M5Unified`
- `M5GFX`
- `Adafruit_NeoPixel`
- `MFRC522_I2C`

En Arduino IDE, abre el `.ino` deseado y selecciona la placa **M5Cardputer**.
El identificador usado por Arduino CLI es:

```text
m5stack:esp32:m5stack_cardputer
```

La [guía Arduino oficial del Cardputer ADV](https://docs.m5stack.com/en/core/Cardputer-Adv)
explica cómo instalar el soporte de la placa. Este repositorio incluye además
un script para quien ya tenga `arduino-cli` y las librerías instaladas:

```bash
./sube.sh L1_Hola                 # compila, sube y abre el monitor serie
./sube.sh L3_HexArcoiris --solo   # solo compila, sin placa conectada
./sube.sh E1_EscapeRoom           # carga el primer juego completo
```

Cada carga sustituye el programa anterior de la placa. El código fuente del
repositorio no se pierde: basta volver a cargar otro sketch cuando se quiera
cambiar de lección o de juego.

## Conexiones usadas

| Conexión | Pines o colores |
|---|---|
| Port A Grove | negro = GND, rojo = 5 V, amarillo = `G2`, blanco = `G1` |
| I²C externo del Port A | `SDA = G2`, `SCL = G1` |
| HEX cuando se usa solo | Port A; señal de datos por `G2` |
| RFID2 | Port A; I²C en `G2/G1`, dirección `0x28` |
| HEX junto con RFID2 | `G4 + GND + 5VOUT` en el conector EXT |
| Botón programable | `G0` |

Otros pines integrados útiles:

| Función | Pines |
|---|---|
| I²C interno: teclado, IMU y códec | `SDA = G8`, `SCL = G9` |
| microSD | `CLK=G40`, `MOSI=G14`, `MISO=G39`, `CS=G12` |
| altavoz I²S | `BCK=G41`, `WS=G43`, `DOUT=G42` |
| emisor infrarrojo | `G44` |

### Conflicto entre HEX y RFID2

El RFID2 habla I²C por `G2/G1`. El HEX no es I²C: utiliza `G2` como línea de
datos para sus LED. Si ambos se conectan a un Hub pasivo, las señales del HEX
interfieren con el bus I²C.

Para usarlos a la vez, los juegos dejan el **RFID2 en el Port A** y conectan el
**HEX a `G4 + GND + 5VOUT` del EXT**. Sus sketches ya están configurados así:

```cpp
#define PIN_HEX 4
```

`L3_HexArcoiris`, al utilizar únicamente el HEX, conserva `PIN_HEX 2` para
poder conectarlo directamente al Port A.

## Alimentación y cuidados

- Mantén bajo el brillo del HEX, especialmente cuando el Cardputer funcione
  con batería.
- En el selector del Port A, `5VOUT` hace que el Cardputer alimente el módulo;
  es la opción utilizada para jugar con batería.
- El interruptor lateral controla la batería, pero el USB puede mantener la
  placa encendida. Para cargar la batería, el interruptor debe estar en `ON`.
- Usa un cable USB-C que transmita datos: algunos cables solo sirven para
  cargar.
- No escribas tarjetas RFID que tengan datos importantes. El escritor de este
  repositorio evita las zonas críticas, pero no puede adivinar qué datos de
  usuario quieres conservar.

## Licencia y atribuciones

El proyecto se publica bajo la [licencia MIT](LICENSE). Puedes usar, copiar,
modificar y redistribuir el código, también en proyectos privados o
comerciales. La condición principal es conservar el aviso de copyright y el
texto de la licencia. El software se entrega sin garantía y sus autores no
asumen responsabilidad por daños derivados de su uso.

Algunos sketches parten de ejemplos oficiales de M5Stack publicados también
bajo MIT. Sus cabeceras conservan el origen, autoría y licencia correspondientes.
