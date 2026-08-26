# Tablero de las tres tarjetas

Este diseño está pensado para copiarse rápidamente en una hoja blanca. Dibuja
tres rectángulos algo mayores que una tarjeta y píntalos antes de jugar:

- Hueco A: rojo.
- Hueco B: azul.
- Hueco C: verde.

No los numeres y no expliques a los jugadores por qué tienen esos colores.

```text
┌──────────────────────┐  ┌──────────────────────┐
│       HUECO A        │  │       HUECO B        │
│                      │  │                      │
│   COLOCA AQUÍ UNA    │  │   COLOCA AQUÍ UNA    │
│       TARJETA        │  │       TARJETA        │
│                      │  │                      │
│ Figura: __________   │  │ Figura: __________   │
│ Cifra:  __________   │  │ Cifra:  __________   │
└──────────────────────┘  └──────────────────────┘

             ┌──────────────────────┐
             │       HUECO C        │
             │                      │
             │   COLOCA AQUÍ UNA    │
             │       TARJETA        │
             │                      │
             │ Figura: __________   │
             │ Cifra:  __________   │
             └──────────────────────┘

┌────────────────────────────────────────────────┐
│ MENSAJE DE LAS FIGURAS:                        │
│                                                │
│ [__________] → [__________] → [__________]     │
│                                                │
│ ¿Qué cuenta? _______________________________   │
│                                                │
│ CÓDIGO FINAL: [       ][       ][       ]      │
└────────────────────────────────────────────────┘
```

Los huecos pueden estar en cualquier disposición. Su colocación física no debe
mostrar cuál se lee primero; esa relación tendrán que descubrirla jugando.
