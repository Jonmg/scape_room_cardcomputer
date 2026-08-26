# Misión: El laboratorio de las luces

**Equipo:** ____________________  **Fecha:** ____________________

El laboratorio se ha bloqueado. Para escapar tendréis que entender sus
luces, responder con el teclado y encontrar tres llaves electrónicas.

## Vuestro equipo

Repartid las funciones. Podéis cambiarlas después de cada prueba.

- **Observador/a:** mira atentamente las luces.
- **Anotador/a:** escribe colores, números y pistas.
- **Operador/a:** utiliza el teclado del Cardputer.
- **Explorador/a:** busca y acerca las tarjetas al lector.

## Reglas del laboratorio

1. Seguid siempre lo que diga la pantalla.
2. No desconectéis cables ni toquéis los pines.
3. No mantengáis pulsado el botón `G0`: reiniciaría la partida.
4. No probéis todas las tarjetas al azar; resolved primero cada pista.
5. Podéis hablar, comparar ideas y volver a observar la secuencia.

## Prueba 1 — El idioma de las luces

1. Pulsad `ENTER` para comenzar.
2. La pantalla muestra qué número significa cada color.
3. El HEX repite una secuencia de **cuatro destellos**.
4. Observad al menos dos repeticiones antes de contestar.
5. Sustituid cada color por su número y escribid las cuatro cifras.

| Destello | Color observado | Número correspondiente |
|---:|---|---:|
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |

**Nuestro código:** `[ _ ][ _ ][ _ ][ _ ]`

- Escribid las cifras con el teclado.
- `DEL` borra la última cifra.
- `ENTER` comprueba el código.
- Si falla, no pasa nada: volved a observar y comparad las notas.

## Prueba 2 — Las tres llaves invisibles

Cada tarjeta tiene un identificador único llamado **UID**. No se ve a simple
vista, pero el lector RFID puede reconocerlo.

Para cada nivel:

1. Leed la pista completa en voz alta.
2. Decidid juntos dónde buscar.
3. Encontrad una tarjeta.
4. Acercadla plana al lector durante un segundo.
5. Esperad el sonido y el mensaje de la pantalla.

Si la pantalla se pone verde, habéis abierto el nivel. Si se pone roja,
revisad la pista y probad con otra tarjeta.

| Nivel | ¿Dónde estaba la tarjeta? | UID mostrado |
|---:|---|---|
| 1 | | |
| 2 | | |
| 3 | | |

## Cuando escapéis

No desmontéis todavía el circuito. Contestad juntos:

1. ¿Qué partes del juego daban información?
2. ¿Qué partes recibían vuestras respuestas?
3. ¿Cómo supo el programa que podía pasar al siguiente nivel?
4. ¿Qué cambiaríais para que la próxima partida fuera más difícil?

### Palabras secretas de programación

- **Entrada:** teclado y lector RFID.
- **Proceso:** comparar vuestra respuesta con la solución.
- **Salida:** pantalla, sonidos y HEX.
- **Escena:** la fase del juego en la que se encuentra el programa.
- **UID:** el identificador único de una tarjeta.
