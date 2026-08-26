# Guía de preparación — Las tres tarjetas de colores

- **Edad:** 9–14 años.
- **Jugadores:** 2–6.
- **Duración:** 20–35 minutos.
- **Hardware:** Cardputer ADV, RFID2, HEX y tres tarjetas A073.

Antes de preparar el material, lee `HISTORIA_PARA_CONTAR.md`. Contarás su
primera parte al comenzar y validarás oralmente el mensaje de las figuras.

## 1. Cableado

Con la placa apagada y el USB desconectado:

- RFID2 → Port A.
- HEX amarillo → `G4` del EXT.
- HEX negro → `GND` del EXT.
- HEX rojo → `5VOUT` del EXT.
- HEX blanco → sin conectar y aislado.
- Selector del Port A → `5VOUT`.

## 2. Preparar el papel

Copia el modelo de `TABLERO_TARJETAS.md` en una hoja blanca:

- pinta el hueco A de rojo;
- pinta el hueco B de azul;
- pinta el hueco C de verde;
- no escribas números en los huecos.

Copia o imprime también estas dos hojas, pero no las entregues al comenzar:

- `PRUEBA_ORDEN_COLORES.md`: escóndela en el salón, cerca de una lámpara.
- `TABLA_CODIGOS_HEX.md`: escóndela en la cocina.

La Cardputer guiará su búsqueda. La primera hoja produce esta relación:

```text
VERDE = posición 1
ROJO  = posición 2
AZUL  = posición 3
```

Puedes cambiarla más adelante modificando `ORDEN_COLORES` en el programa.

No expliques que esta relación determina cómo colocar o leer las tarjetas. Los
huecos pintados y las figuras del HEX deben permitir que lo deduzcan.

## 3. Tarjetas necesarias

El escritor `L5_RfidEscribir` quedará preparado con estos textos:

| Tecla | Texto RFID | Color | Figura | Cifra de la tabla |
|---:|---|---|---|---:|
| `1` | `E3\|RO\|FLECHA` | Rojo | Flecha | 4 |
| `2` | `E3\|AZ\|ANILLO` | Azul | Anillo | 8 |
| `3` | `E3\|VE\|X` | Verde | X | 9 |

La programación física de estas tarjetas se hará después de probar el
firmware principal.

## 4. Escondites y orden de búsqueda

El orden de búsqueda está mezclado respecto al código final:

| Paso | Escondite | Objeto |
|---:|---|---|
| 1 | Salón, cerca de una lámpara | Prueba de las tres luces |
| 2 | Donde están los libros | Tarjeta roja / Flecha |
| 3 | Donde están los CDs | Tarjeta azul / Anillo |
| 4 | Donde están los juguetes | Tarjeta verde / X |
| 5 | Cocina | Codificador de objetos a cifras |

El Cardputer solo acepta la tarjeta correspondiente a la pista actual.

## 5. Solución

Después de leerlas, el tablero contiene:

| Color | Figura del HEX | Cifra |
|---|---|---:|
| Rojo | Flecha roja | 4 |
| Azul | Anillo azul | 8 |
| Verde | X verde | 9 |

La prueba anterior ordena los colores así:

```text
1. VERDE → 9
2. ROJO  → 4
3. AZUL  → 8
```

Las figuras en ese mismo orden son `X → FLECHA → ANILLO`. La lectura prevista
es «desde la X, sigue la flecha hasta el portal»; acepta interpretaciones
próximas que utilicen los tres símbolos. El código final es **948**.

## 6. Ensayo

1. Sube `E3_EscapeRoomAvanzado`.
2. Mantén `G0` tres segundos y suéltalo para empezar desde cero.
3. Pulsa `ENTER`, comprueba la búsqueda de la prueba de luces y vuelve a pulsar
   `ENTER` cuando simules haberla encontrado.
4. Cuando las tarjetas estén programadas, lee roja, azul y verde.
5. Confirma que el HEX mantiene flecha roja, anillo azul y X verde hasta
   pulsar `ENTER`.
6. Comprueba que después de la tercera tarjeta obliga a buscar el codificador.
7. Comprueba después `MENSAJE OCULTO` y que otro `ENTER` abre el teclado.
8. Introduce `948` y comprueba la pantalla de victoria y el arcoíris.

La primera prueba real del HEX es importante: el dibujo utiliza la numeración
por filas del esquema del NeoHEX. Si alguna figura aparece deformada, habrá que
ajustar el mapa de píxeles después de observar la placa.

## 7. Personalización

El bloque `🔧 CAMBIA ESTO` del `.ino` concentra:

- título, introducción y mensaje final;
- escondites y pistas de las tarjetas y las dos hojas;
- textos, colores y figuras de las tarjetas;
- `ORDEN_COLORES`, que determina el número final;
- pines y brillo.

La cifra no está guardada en la tarjeta: procede de `SIMBOLOS`. Esto mantiene
separadas las tres reglas del acertijo.
