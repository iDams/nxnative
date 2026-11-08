# Reglas del proyecto — Zelda: Ocarina of Time (iOS/Mac)

Estas reglas son obligatorias para cualquier agente o desarrollador que toque este proyecto. El handoff técnico está en [`HANDOFF_ZELDA_OOT_AI.md`](HANDOFF_ZELDA_OOT_AI.md).

## 1. Icono de la app

- El icono es **`/Users/marco/Ocarina/assets/icon.jpeg`** (copia de `/Users/marco/Downloads/images.jpeg`, 554×554, arte de Ocarina of Time).
- Al preparar el proyecto Xcode o empaquetar la app, pasar siempre:
  ```bash
  DM_ICON=/Users/marco/Ocarina/assets/icon.jpeg
  ```
- **No usar** `extract/v131072/icon.jpg` ni el icono por defecto del juego: es el que queda mal.
- El icono se copia al asset catalog `Gen/Assets.xcassets/AppIcon.appiconset/icon.png` (1024×1024) al regenerar el proyecto. No editar `Gen/` a mano: se regenera en cada `build_ipa.sh`.
- Si se cambia la imagen, reemplazar `assets/icon.jpeg`, regenerar el proyecto y reinstalar la app.

## 2. Espacio y limpieza (no dejar basura)

- Mantener **al menos 20 GB libres** en `/System/Volumes/Data`. Comprobar con `df -h` antes y después de compilar.
- Después de cada build/prueba, borrar lo que ya no se use:
  - Bibliotecas de shaders de cadenas viejas (`shader/ios/b-<otra>.img`, `shader/mac/b-<otra>.img`, sus `cas.db`, `inputs.tsv` viejos).
  - `xcode-derived-data`, `.app`/IPA viejos, `build/ios` de una cadena anterior.
  - Módulos `patched_*` viejos (conservar solo el de la cadena/perfil actual).
  - Backups temporales duplicados, logs gigantes y `/tmp` de pruebas.
- **Nunca borrar**:
  - `Documents/RomFS.img` del iPhone ni `extract/v131072/RomFS.img` del Mac.
  - `save/`, `save_mac/` ni `Documents/save` del iPhone.
  - El perfil actual (`dm/games/0400486024DEC000/`), `shader/pkg_5636`, `shader/tables_5636`, `patched_37f` y el handoff.
- **Nunca volver a copiar el RomFS de 24,4 GB**: ya está en el iPhone. La app se instala encima y usa `Documents/RomFS.img`.
- Al empaquetar: `DM_COPY_ROMFS=NO` (la app no lleva el RomFS dentro).
- Los datos del juego, claves y extractos viven en el disco externo o en `/Users/marco/Ocarina`, nunca dentro de `/Users/marco/ns` ni en el home.
- Los backups y notas son `.md`/texto; no duplicar binarios grandes.

## 3. Instalación en el iPhone

- Instalar **encima** de la app existente, sin desinstalar:
  ```bash
  xcrun devicectl device install app --device 00008150-00190C880E3B401C \
    /ruta/al/DmIOS.app
  ```
- Desinstalar borra `Documents/RomFS.img` y las partidas. **No hacerlo.**
- El bundle ID es `com.marcoastorga.zeldaoot`; el equipo `TGU3UHC7C7`.
- La app debe llevar `com.apple.developer.kernel.increased-memory-limit` y `DM_GUEST_FILE=1` (ya forzado en `main.m`).

## 4. Build y rendimiento

- Perfil actual: `0400486024DEC000`, cadena `39417042cfe8`, `fps_target 60`, `op_mode handheld`, `DM_GUEST_FILE=1`, corte NDVE en `patches.py`.
- iOS: biblioteca `.img` en `shader/ios/b-<chain>.img`; Mac: `shader/mac/`. No mezclar cadenas.
- La biblioteca instalada es la **completa** (42.848 shaders, `shader/ios/b-39417042cfe8.img`, 5,4 GB, sello `inputs 4bd48f723876` dentro de la imagen). No sustituirla por el antiguo set parcial de la ruta: causaba `failed to load shader` y caídas a ~7 fps.
- El frame step puede bajar a 20/30 fps por coste de encode CPU (~18,5 ms) aunque la GPU tenga margen; no confundirlo con shaders ausentes. Medir con el log (`timing up to frame`, `frame step`).
- Antes de empaquetar, el `c1_full.tsv`/`cb_extent.tsv` debe nombrar la cadena y el `inputs` de la biblioteca; si no, el host usa 64 KiB por c1 y pierde rendimiento.
- La app del iPhone no debe llevar el RomFS; el Mac no se usa para jugar en el teléfono.

## 5. Documentación

- La sesión del **2026-11-08** está documentada en [`DOCUMENTACION_SESION_2026-11-08.md`](DOCUMENTACION_SESION_2026-11-08.md) (copias en `/Users/marco/ns` y `/Users/marco/Ocarina`): historial, estado de la instalación, causa de los 7 fps y pendientes.
- Mantener actualizado `HANDOFF_ZELDA_OOT_AI.md` (una copia en `/Users/marco/ns` y otra en `/Users/marco/Ocarina`) con: commit del motor, cadena de shaders, perfil, rutas, memoria medida, problemas abiertos y comandos.
- Cada cambio de perfil, shader o host que afecte al juego se anota en la nota `dm/games/0400486024DEC000/README.md`.
- No dejar notas temporales sueltas; van al handoff o a este archivo.
