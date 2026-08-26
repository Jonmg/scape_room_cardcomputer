# E3 — Las tres tarjetas de colores

Escape room para Cardputer ADV, Unit RFID2, Unit HEX y tres tarjetas A073.

La mecánica separa claramente tres informaciones:

1. La **tarjeta** proporciona un color y una figura.
2. La **tabla codificadora** convierte la figura en una cifra.
3. Una **prueba anterior** relaciona los colores con las posiciones 1, 2 y 3.

Cada tarjeta hace que el HEX dibuje su figura usando su propio color. La
Cardputer hace buscar la prueba de las luces antes de las tarjetas y oculta el
codificador de cifras hasta después de la tercera. No explica cómo relacionar
las piezas: los jugadores deben deducirlo observando las hojas y los huecos
pintados. Las figuras forman además un pequeño mensaje que deben interpretar y
contar al organizador antes de introducir el número.

## Archivos

- `E3_EscapeRoomAvanzado.ino`: programa de la partida.
- `GUIA_PREPARACION.md`: montaje, tarjetas, escondites y solución.
- `GUIA_JUGADORES.md`: instrucciones sin la solución.
- `HISTORIA_PARA_CONTAR.md`: narración y respuestas válidas para el adulto.
- `PRUEBA_ORDEN_COLORES.md`: acertijo previo que determina las posiciones.
- `TABLA_CODIGOS_HEX.md`: seis figuras que el HEX puede representar.
- `TABLERO_TARJETAS.md`: modelo sencillo para copiar en una hoja blanca.
- `IDEAS_PARA_AMPLIAR.md`: ampliaciones para partidas futuras.

El escritor está fuera de esta carpeta, en `../L5_RfidEscribir`.

## Configuración actual

| Tarjeta | Color | Figura | Escondite |
|---|---|---|---|
| `E3\|RO\|FLECHA` | Rojo | Flecha | Libros |
| `E3\|AZ\|ANILLO` | Azul | Anillo | CDs |
| `E3\|VE\|X` | Verde | X | Juguetes |

La prueba anterior establece `VERDE=1`, `ROJO=2`, `AZUL=3`. Con la tabla
actual, las figuras ordenadas cuentan «desde la X, sigue la flecha hasta el
portal» y la solución numérica es `948`.

## Compilar

```bash
./sube.sh E3_EscapeRoomAvanzado --solo
```
