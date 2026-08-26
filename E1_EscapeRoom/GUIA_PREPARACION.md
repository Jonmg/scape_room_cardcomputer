# Guía de preparación — Partida tutorial

## El laboratorio de las luces

Esta es una partida corta para aprender cómo funciona el Cardputer antes de
diseñar un escape room más difícil.

- **Edad:** 9–14 años.
- **Jugadores:** 2–5.
- **Duración:** 10–15 minutos más una explicación final.
- **Objetivo didáctico:** reconocer entrada, proceso, salida y escenas.
- **Firmware:** `E1_EscapeRoom` en modo demo.

La partida tiene cuatro pasos:

1. Observar una secuencia en el HEX.
2. Sustituir cada color por el número indicado en pantalla.
3. Introducir el código con el teclado.
4. Resolver tres pistas y presentar tres tarjetas RFID diferentes.

---

## 1. Material necesario

- [ ] Cardputer ADV con batería cargada.
- [ ] Unit HEX.
- [ ] Unit RFID2.
- [ ] Tres tarjetas o llaveros RFID diferentes.
- [ ] Cable Grove del RFID2.
- [ ] Cableado del HEX hacia el conector EXT.
- [ ] Un reloj, varios libros y una mesa o silla para los escondites.
- [ ] Papel y lápiz para los niños.
- [ ] La hoja `GUIA_NINOS.md` impresa o copiada.

Opcionalmente, marca las tarjetas por detrás como `1`, `2` y `3`. Esas
marcas son para la persona que prepara la partida y no deberían ser visibles
desde su escondite.

## 2. Montaje electrónico

Haz todo el cableado con el Cardputer apagado y sin USB.

### RFID2

Conecta el RFID2 directamente al **Port A** mediante su cable Grove.

### HEX

Conecta el HEX al conector EXT:

| Cable del HEX | Cardputer ADV | EXT |
|---|---|---:|
| Amarillo, señal | `G4` | pin 3 |
| Negro | `GND` | pin 4 |
| Rojo | `5VOUT` | pin 6 |
| Blanco | Sin conectar | — |

No uses `5VIN` para alimentar el HEX desde la batería. Comprueba dos veces
`GND` y `5VOUT` antes de encender.

Pon el selector de alimentación del Port A en **`5VOUT`** y el interruptor
lateral en **`ON`**. Para la partida es preferible usar la batería y retirar
el USB, para que los niños no tropiecen con el cable.

## 3. Configuración del programa

La partida tutorial debe tener estos valores:

```cpp
const bool CAMBIO_MANUAL_MODULOS = false;
#define PIN_HEX 4
const bool MODO_DEMO_UID = true;
```

El modo demo acepta tres tarjetas distintas cualesquiera. No hace falta
conocer sus UID antes de jugar.

Compila y sube el juego:

```bash
cd ~/Arduino/CardputerLab
./sube.sh E1_EscapeRoom
```

Si aparece una escena de una partida anterior, mantén pulsado `G0` durante
tres segundos y suéltalo cuando la pantalla lo pida.

## 4. Preparación de los escondites

El programa muestra las pistas en este orden:

| Tarjeta | Escondite preparado | Pista que aparece |
|---:|---|---|
| 1 | Cerca de un reloj | “Busca cerca de algo que marca el tiempo.” |
| 2 | Entre libros o junto a una estantería | “Busca donde viven muchas historias.” |
| 3 | Bajo una mesa o silla estable | “Busca bajo algo con patas que no camina.” |

No escondas tarjetas cerca de enchufes, líquidos, ventanas, objetos frágiles
o lugares a los que haya que subirse. Deben poder encontrarlas sin mover
muebles pesados.

En modo demo las identidades aún no están fijadas: lo importante es que las
tres tarjetas sean diferentes y que cada escondite contenga solo una.

## 5. Solución para la persona que organiza

Esta sección contiene spoilers.

La pantalla muestra esta sustitución:

| Color | Número |
|---|---:|
| Rojo | 7 |
| Verde | 3 |
| Azul | 1 |
| Amarillo | 9 |

El HEX repite:

```text
AZUL → VERDE → ROJO → AMARILLO
```

Por tanto, el código correcto es **1379**.

## 6. Ensayo completo antes de llamar a los niños

1. Enciende el Cardputer y comprueba que aparece `LABORATORIO SECRETO`.
2. Pulsa `ENTER` y observa dos ciclos completos del HEX.
3. Escribe `1379` y pulsa `ENTER`.
4. Comprueba que aparece `LLAVE RFID 1 DE 3`, sin pedir un cambio de módulo.
5. Presenta la tarjeta 1; debe verse `LLAVE CORRECTA` y su UID.
6. Repite con las tarjetas 2 y 3. No reutilices una tarjeta.
7. Comprueba la escena `ESCAPE COMPLETADO!` y el arcoíris final del HEX.
8. Mantén `G0` tres segundos, suéltalo y déjalo de nuevo en la introducción.
9. Esconde las tarjetas. La partida ya está lista.

## 7. Cómo presentar la partida tutorial

Antes de comenzar, dedica dos minutos a enseñar los componentes sin revelar
la solución:

- La **pantalla** da instrucciones.
- El **HEX** es una salida: el programa manda colores.
- El **teclado** es una entrada: el equipo responde al programa.
- El **RFID2** es otra entrada: reconoce el UID de cada tarjeta.
- Cada prueba superada cambia la **escena** del programa.

Reparte cuatro funciones. Si hay menos niños, una persona puede asumir dos:

- Observador de luces.
- Anotador.
- Operador del teclado.
- Explorador de tarjetas.

Conviene rotar las funciones después del código para que todos participen.
Durante la partida, ayuda con preguntas —“¿qué color fue primero?”— en lugar
de dar directamente la respuesta.

## 8. Problemas frecuentes

### El HEX no se ilumina

- Comprueba `PIN_HEX = 4`.
- Revisa `G4`, `GND` y `5VOUT` con la placa apagada.
- Verifica que el hilo blanco esté aislado y sin tocar otros pines.

### El código no funciona

- Espera dos secuencias completas.
- Comprueba que se introdujeron exactamente cuatro cifras.
- `DEL` borra la última cifra y `ENTER` comprueba.
- Para esta partida la solución del organizador es `1379`.

### Aparece “NO VEO EL RFID2”

- Comprueba que el RFID2 está en Port A.
- Pon el selector del Port A en `5VOUT`.
- Apaga, revisa el Grove y vuelve a encender.
- Si continúa, prueba primero `L4_RfidLeer`.

### Una tarjeta es rechazada

En modo demo no se puede usar la misma tarjeta dos veces. Retírala del
lector y utiliza otra diferente.

### El juego comienza por la mitad

El progreso se guarda aunque se apague. Mantén `G0` tres segundos y suéltalo
cuando la pantalla diga `Suelta G0...`.

## 9. Después de la prueba

Anota los UID que aparecen en pantalla o en el monitor serie:

| Tarjeta | UID |
|---:|---|
| 1 | |
| 2 | |
| 3 | |

Para la siguiente versión podremos fijar esos UID, exigir un orden concreto,
añadir pistas falsas, tiempo, penalizaciones y una historia más elaborada.
