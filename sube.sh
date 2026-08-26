#!/usr/bin/env bash
# Compila, sube y abre el monitor serie.
#   ./sube.sh L3_HexArcoiris          -> compila, sube y monitoriza
#   ./sube.sh L3_HexArcoiris --solo   -> solo compila (sin placa)
set -euo pipefail

FQBN="m5stack:esp32:m5stack_cardputer"
CLI="${HOME}/bin/arduino-cli"
cd "$(dirname "$0")"

if [ $# -lt 1 ]; then
  echo "Uso: ./sube.sh <carpeta_del_sketch> [--solo]"
  echo "Disponibles:"; ls -d L*/ E*/ 2>/dev/null | tr -d '/' | sort | sed 's/^/  /'
  exit 1
fi
SKETCH="${1%/}"

echo "▸ Compilando $SKETCH ..."
"$CLI" compile -b "$FQBN" "$SKETCH"

if [ "${2:-}" = "--solo" ]; then echo "✔ Solo compilacion."; exit 0; fi

PUERTO=$(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null | head -1 || true)
if [ -z "$PUERTO" ]; then
  echo "✖ No encuentro la placa. Conecta el Cardputer por USB-C y reintenta."
  exit 1
fi

echo "▸ Subiendo a $PUERTO ..."
"$CLI" upload -b "$FQBN" -p "$PUERTO" "$SKETCH"
echo "✔ Subido. Monitor serie (Ctrl-C para salir):"
sleep 2
"$CLI" monitor -p "$PUERTO" -c baudrate=115200
