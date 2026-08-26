# Guía de preparación — El misterio de Juanita

## Las cuatro memorias

Juanita es un alma buena que está en el cielo y quiere ayudar a su familia.
La casa parece llena de sombras y susurros porque cuatro de sus recuerdos se
han perdido. El equipo deberá recuperarlos y ordenar su mensaje final.

La historia utiliza el misterio y los fantasmas de manera amable: Juanita no
es el peligro, sino quien guía a los jugadores. Antes de jugar, confirma que
este enfoque resulta agradable para toda la familia.

- **Edad:** 9–14 años.
- **Jugadores:** 2–6.
- **Duración estimada:** 20–30 minutos.
- **Firmware:** `E2_MisterioJuanita`.
- **Tarjetas RFID:** cuatro diferentes.

## 1. Cómo funciona la partida

El Cardputer permanece en el salón como “mesa de comunicación”. La pantalla
muestra un acertijo que conduce directamente a uno de los cuatro dormitorios,
donde está escondida una tarjeta RFID. Los niños llevan la tarjeta al salón,
la presentan al lector y descubren una memoria formada por un color y un
número. No se utilizan notas de papel escondidas.

Después de reunir las cuatro memorias, el HEX muestra los colores en un orden
nuevo. Sustituyendo cada color por su número se obtiene el código final.

## 2. Materiales

- [ ] Cardputer ADV cargado.
- [ ] Unit RFID2 conectado al Port A.
- [ ] Unit HEX conectado al EXT.
- [ ] Cuatro tarjetas o llaveros RFID diferentes.
- [ ] Una hoja `GUIA_NINOS_E2.md` por equipo.
- [ ] Papel, lápiz y cinta adhesiva que no dañe muebles.

Marca discretamente las tarjetas por detrás como `1`, `2`, `3` y `4`. Estas
marcas solo sirven para preparar la partida.

## 3. Cableado

Con la placa apagada y el USB desconectado:

- RFID2 → Port A mediante Grove.
- HEX amarillo → `G4` del EXT, pin 3.
- HEX negro → `GND` del EXT, pin 4.
- HEX rojo → `5VOUT` del EXT, pin 6.
- HEX blanco → sin conectar y aislado.

Pon el selector del Port A en `5VOUT` y el interruptor lateral en `ON`.

La configuración del programa debe ser:

```cpp
const bool CAMBIO_MANUAL_MODULOS = false;
#define PIN_HEX 4
const bool MODO_DEMO_UID = true;
```

## 4. Ruta por la casa

El Cardputer y el lector se quedan en el **salón** durante toda la partida.

| Paso | Pista de la pantalla | Tarjeta escondida |
|---:|---|---|
| 1 | Donde los juguetes esperan compañía | Dormitorio de juguetes |
| 2 | Donde las historias duermen de pie | Dormitorio de libros |
| 3 | Junto a círculos que guardan música | Dormitorio de CD |
| 4 | Donde Juanita pasaba un buen rato | Habitación de Juanita |

La cocina y los dos baños quedan como posibles lugares falsos. Esto obliga a
resolver el acertijo en lugar de registrar toda la casa al azar. El salón es
la base y los cuatro dormitorios contienen las cuatro llaves.

Las frases pueden cambiarse en `PISTAS_RUTA`, dentro del bloque
`🔧 CAMBIA ESTO`, si queréis hacerlas más misteriosas o difíciles.

## 5. Colocación exacta

1. Esconde la tarjeta 1 en el dormitorio de juguetes.
2. Esconde la tarjeta 2 en el dormitorio de libros.
3. Esconde la tarjeta 3 en el dormitorio de CD.
4. Esconde la tarjeta 4 en un sitio respetuoso y fácil de alcanzar de la
   habitación donde Juanita pasaba tiempo.
5. Coloca el Cardputer, el HEX y el RFID2 en una mesa estable del salón.

No escondas nada dentro de aparatos eléctricos, detrás de enchufes, junto a
medicinas o productos de limpieza, ni donde haya que trepar o mover muebles.

## 6. Solución del organizador

Cada tarjeta revela estas memorias:

| Tarjeta | Memoria | Número |
|---:|---|---:|
| 1 | Roja | 4 |
| 2 | Azul | 8 |
| 3 | Verde | 2 |
| 4 | Amarilla | 6 |

Al final el HEX destella:

```text
VERDE → ROJO → AMARILLO → AZUL
```

Sustituyendo colores por los números apuntados:

```text
VERDE=2, ROJO=4, AMARILLO=6, AZUL=8
```

El código final es **2468**.

## 7. Subida y ensayo

Sube el programa:

```bash
cd ~/Arduino/CardputerLab
./sube.sh E2_MisterioJuanita
```

Realiza una partida completa antes de esconderlo todo:

1. Comprueba que aparece `MISTERIO DE JUANITA`.
2. Pulsa `ENTER` y lee el primer susurro.
3. Presenta la tarjeta 1 y apunta `ROJA = 4`.
4. Continúa con las otras tres tarjetas en orden.
5. Observa dos secuencias completas del HEX.
6. Introduce `2468` y pulsa `ENTER`.
7. Comprueba `MISTERIO RESUELTO`, el mensaje final y el arcoíris.
8. Mantén `G0` durante tres segundos y suéltalo para borrar el ensayo.
9. Coloca las cuatro tarjetas en sus escondites.

## 8. Dirección del juego

Explica solo estas ideas antes de empezar:

- Juanita intenta ayudar; los otros susurros han desordenado sus recuerdos.
- El Cardputer se queda en el salón.
- Cada pista de la pantalla debe resolverse antes de probar una tarjeta.
- Cada memoria revelada debe anotarse porque hará falta al final.

Si se bloquean, ofrece ayudas progresivas:

1. Lee de nuevo exactamente lo que dice la pantalla.
2. Pregunta qué palabras importantes han encontrado.
3. Recuerda qué habitaciones quedan sin visitar.
4. Solo como última ayuda, señala la estancia correcta.

## 9. Problemas frecuentes

### El juego empieza por la mitad

Mantén `G0` tres segundos y suéltalo cuando aparezca `Suelta G0...`.

### No aparece el RFID2

Revisa el Grove, el Port A y su selector `5VOUT`. Prueba `L4_RfidLeer` si
continúa sin responder.

### Una tarjeta es rechazada

En modo demo cada tarjeta solo puede revelar una memoria. Usa cuatro tarjetas
diferentes y retira cada una del lector después de escanearla.

### El HEX no muestra el código final

Revisa `PIN_HEX = 4` y el cableado `G4 + GND + 5VOUT`.

## 10. UID para la versión definitiva

Durante el ensayo, anota los UID mostrados:

| Tarjeta | UID |
|---:|---|
| 1 | |
| 2 | |
| 3 | |
| 4 | |

Después se pueden copiar en `UID_TARJETAS` y cambiar
`MODO_DEMO_UID = false`. Así el programa exigirá las tarjetas concretas en
el orden correcto.
