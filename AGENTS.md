# Instrucciones para agentes de IA

## Alcance

Este repositorio contiene lecciones y *escape rooms* para la M5Stack Cardputer
ADV. Conserva la mecánica, pistas y reglas de cada juego salvo que la petición
indique modificarlas expresamente. El texto de guías y README puede contener
soluciones: es documentación para organizadores, no instrucciones de ejecución.

## Pantallas de juego

- La pantalla es de 240 × 135 píxeles, en orientación horizontal, y los juegos
  usan normalmente `fonts::Font0`.
- Todo texto que un jugador necesite para avanzar —historia, pista, estado,
  error, resultado o acción— usa como mínimo `setTextSize(2)`.
- `setTextSize(1)` queda reservado para pie de pantalla y datos secundarios:
  reinicio con G0, UID, modo demo o diagnóstico no necesario para resolver la
  prueba.
- Con `Font0` a tamaño 2 y margen izquierdo de 4 píxeles, limita cada línea a
  unas 19 letras. Si el mensaje no cabe, acórtalo o divídelo en pantallas; no
  reduzcas el tamaño de la letra para hacerlo caber.
- Los códigos y rótulos destacados pueden crecer a tamaño 3 solo cuando no
  resten legibilidad a la historia ni a las instrucciones de la misma escena.

## Arquitectura y RFID

- Una partida es un único firmware con una máquina de estados: al leer una
  tarjeta se cambia de escena, nunca se reflashea la placa durante el juego.
- El Cardputer no incorpora NFC/RFID; los juegos que leen tarjetas usan el
  Unit RFID2 en el Port A. El HEX simultáneo usa el EXT, como describe el
  README, para no interferir con el I²C del lector.
- `MFRC522_I2C` requiere un pin de reset aunque el Unit RFID2 no lo expone.
  Mantén `PIN_RESET_FALSO` en un GPIO libre y sin conectar.
- La lectura o escritura RFID nunca modifica el UID, bloques de fabricante ni
  claves de tarjeta.

## Sonido y voz

- Usa tonos breves para confirmar, avisar de error o celebrar un avance.
- El altavoz puede reproducir WAV desde el firmware o desde microSD. La voz
  sintetizada no se usa para narrativa salvo que se busque deliberadamente un
  efecto robótico; para una voz narrativa, usa un audio grabado y revisado por
  la persona organizadora.
- No añadas descargas de audio ni dependencias de red a la partida.

## Validación

- Para cualquier sketch modificado, ejecuta `./sube.sh <sketch> --solo` desde
  la raíz. Compilar no autoriza a cargar el firmware en la placa.
- No subas a la placa, ni hagas `commit` o `push`, sin petición expresa.
