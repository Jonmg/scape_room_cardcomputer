# Preparar tarjetas RFID para E3

Este programa guarda uno de estos textos en cada tarjeta A073:

| Tecla | Texto guardado |
|---:|---|
| `1` | `E3\|RO\|FLECHA` |
| `2` | `E3\|AZ\|ANILLO` |
| `3` | `E3\|VE\|X` |

## Uso

1. Conecta solamente el RFID2 al Port A y ponlo en `5VOUT`.
2. Sube el programa con `./sube.sh L5_RfidEscribir`.
3. Sin una tarjeta sobre el lector, pulsa `1`, `2` o `3`.
4. Comprueba el texto elegido y pulsa `ENTER`.
5. Ahora acerca una tarjeta y no la muevas hasta ver
   `ESCRITURA CORRECTA`.
6. Marca físicamente la tarjeta por detrás para no confundirla durante la
   preparación.
7. Pulsa `R` y vuelve a acercarla si quieres comprobar su contenido.

Cada nueva preparación también borra el sello final que pueda quedar de una
partida anterior.

## Límites y seguridad

- Funciona con MIFARE Classic que conserve la clave de fábrica y con muchas
  Ultralight/NTAG sin protección de escritura.
- Rechaza otros tipos de tarjeta.
- No modifica el UID, las claves ni los bloques de control.
- Sí reemplaza los datos que hubiera en los bloques/páginas de usuario que
  reserva para E3. Usa tarjetas sin información importante.
- Los textos de `TEXTOS_TARJETA` admiten como máximo 15 caracteres ASCII.

Si aparece un error de autenticación, la tarjeta puede usar otra clave o
estar protegida. No insistas con una tarjeta de acceso, transporte o pago;
utiliza las tarjetas en blanco destinadas al juego.
