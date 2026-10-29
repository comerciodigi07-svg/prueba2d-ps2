# PongPS2 — prueba 0.1

Juego casero 2D para PlayStation 2 (homebrew), usando PS2SDK + gsKit.

## Estado real de esta entrega
Este paquete contiene el código fuente y la automatización de compilación. **No incluye todavía un ISO compilado y probado**: el entorno donde se preparó no tiene instalado el compilador de PS2 ni gsKit. El flujo de GitHub Actions incluido compila el ELF y crea una ISO cuando se ejecuta en un entorno con PS2DEV correctamente configurado.

## Controles
- Cruceta arriba/abajo: mover la paleta del jugador.
- START: reiniciar partida.
- SELECT: salir al sistema (si el entorno de ejecución lo permite).

## Compilar
Necesitas un entorno PS2DEV/PS2SDK con gsKit instalado y `mips64r5900el-ps2-elf-gcc`, `make` y `genisoimage` o `mkisofs`.

```bash
make
make iso
```

El ejecutable se genera como `PONGPS2.ELF`; la imagen se genera como `PongPS2.iso`.

## Probar
- PCSX2: usa la opción de ejecutar ELF para una prueba inicial, o monta `PongPS2.iso`.
- PS2 real: requiere un método compatible con homebrew. La ISO por sí sola no elimina las restricciones de arranque de la consola.

## Nota
Este es un prototipo mínimo: marcador visual con dígitos dibujados, rival automático y física sencilla. No incluye música ni recursos externos.
