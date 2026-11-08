# HANDOFF TÉCNICO: The Legend of Zelda: Ocarina of Time (Switch 2 en iOS)

> **Reglas obligatorias del proyecto:** [`REGLAS.md`](REGLAS.md) — icono (`assets/icon.jpeg`), limpieza y espacio (no basura, no duplicar el RomFS), instalación encima de la app y build actual.

> **Documentación de la sesión 2026-11-08:** [`DOCUMENTACION_SESION_2026-11-08.md`](DOCUMENTACION_SESION_2026-11-08.md) — historial del motor, estado exacto de la app instalada (7 fps → 20–30 fps), límite CPU del frame step, comandos de build y pendientes.

> **Documento de relevo y especificación técnica para agentes de IA e ingenieros de software.**  
> Este documento contiene toda la información necesaria para compilar, instalar, depurar y continuar el desarrollo del proyecto *The Legend of Zelda: Ocarina of Time* ejecutándose nativamente sobre iOS mediante recompilación dinámica/estática.

> [!IMPORTANT]
> **Actualización 2026-11-08 (estado actual):** el motor es **`7f502220` + cambios locales + parche NX2Apple 0096**, con la cadena **`39417042cfe8`** e `inputs 4bd48f723876`. `7f502220` trae los render targets de la resolución dinámica en heaps reciclados (evita ~3 texturas nuevas/frame y +455–521 MB de residency por 300 frames), el fix del crash de Kokiri en AGX `prepareForEnqueue` y el controlador nuevo de resolución dinámica. El parche 0096 divide los mesh draws de OoT en grids de 1024 y evita los page faults de GPU de la hierba (hasta 1.771 workgroups). La **biblioteca iOS completa** (42.848 shaders, 5,4 GB) está instalada y ya no hay ningún `failed to load shader`.
> El perfil nuevo añade `fps_target 60` y los stubs de `frame ratio`/`dynamic resolution` (reescritos en `main`); la resolución dinámica queda a **full** por defecto (`DM_DYNAMIC_RESOLUTION=0`). Se conserva el corte NDVE y `op_mode handheld`. El `front_mac` lleva además la suspensión real en standby (sueño, bloqueo, oculto, tapado: `dm_audio_suspend + dm_gfx_suspend`), que antes no existía y dejaba al juego renderizando y creciendo en RAM.
> La app iOS se compila con la biblioteca **completa** `shader/ios/b-39417042cfe8.img` (42.848 shaders, 5,4 GB, sello dentro del `.img`), `c1_full`/`cb_extent` de esa cadena, `PATCHED=patched_7f` y el icono **`/Users/marco/Ocarina/assets/icon.jpeg`**. La app instalada son 5,5 GB. El Mac se eliminó para liberar espacio (el host fuente sigue en `dm/`). La app instalada en el iPhone se actualiza encima, sin tocar `Documents/RomFS.img` ni `save`; no se copia el RomFS. En juegos se midió **20–30 fps**: la GPU va a 14–16,6 ms p50 y el encode CPU a ~18,5 ms, que no cabe en un vsync; `Documents/env.txt` lleva `DM_DYNAMIC_RESOLUTION=1` para probar el controlador nuevo en el siguiente arranque.

---

## 1. Repositorios Base y Dependencias

| Repositorio | URL Remota | Rama / Commit | Rol |
| :--- | :--- | :--- | :--- |
| **nxnative (Fork Principal)** | `https://github.com/iDams/nxnative.git` | `main` | Motor nativo de recompilación para Switch / Switch 2 en Apple Silicon / iOS. Clonado local en `/Users/marco/ns`. |
| **nxnative (Upstream)** | `https://github.com/seedds/nxnative.git` | `main` | Repositorio upstream original del motor `dm`. |
| **NX2Apple (Frontend & Perfiles)** | `https://github.com/LNPANDY/NX2Apple` | `main` (pin `88c7e03f`, Release v1.2.3) | Frontend comunitario y serie de 53 parches sobre nxnative; de aquí salen los parches iOS y el perfil de Ocarina of Time actualizado. |
| **d2r-mobile (Base Histórica)** | `https://github.com/jhaizhou-ops/d2r-mobile` | `main` | Proyecto MIT original del que derivó la técnica de recompilación nativa Switch a Metal/ARM64. |
| **Entorno Local del Motor** | Ruta: `/Users/marco/ns` | Local | Donde reside el código fuente del motor (`dm`). |
| **Entorno del Proyecto Zelda**| Ruta: `/Users/marco/Ocarina` | Local | Workspace con proyecto Xcode, binarios parcheados, shaders y scripts. |

---

## 2. Información del Juego y Plataforma Objetivo

* **Juego:** *The Legend of Zelda: Ocarina of Time* (Nintendo Switch 2 / Ounce).
* **Title ID / Game ID:** `0400486024DEC000`.
* **Versión:** 1.1.1 (`v131072`).
* **SDK Nintendo:** `nnSdk-Generic_22_2_8-Release` (Major 22).
* **Módulos:** `main`, `subsdk0`, `sdk`.
* **Dispositivo Objetivo:** iPhone 17 Pro Max ("iPhone Dams").
  * **Identificador (UDID):** `00008150-00190C880E3B401C`.
  * **Modelo de Hardware:** `iPhone18,2` (Apple A19 Pro GPU, 12 GB LPDDR5X RAM).
* **Bundle Identifier iOS:** `com.marcoastorga.zeldaoot`.
* **Equipo / Firma:** `Apple Development: Marco Astorga (L4J24857AX)`.

---

## 3. Rutas Absolutas Críticas en el Sistema

### A. Repositorio del Motor (`/Users/marco/ns`)
* **`/Users/marco/ns/dm/gfx/nvn/nvn_mtl.m`**: Backend gráfico Metal 4. Gestiona pools de texturas, shaders, swap chains, buffers de staging y residencia de memoria GPU.
* **`/Users/marco/ns/dm/host/hle_svc.c`**: Emulación de syscalls del kernel Horizon (`SetHeapSize`, `MapPhysicalMemory`, gestión del espacio de direcciones virtual).
* **`/Users/marco/ns/dm/host/gen/0400486024DEC000/game_profile.h`**: Perfil de hardware virtual del juego (límites de memoria, tamaño de alias y pools).
* **`/Users/marco/ns/dm/ios/Sources/DmGamepad.m`**: Lógica de controles táctiles en pantalla (joysticks virtuales, botones A/B/X/Y, gatillos).
* **`/Users/marco/ns/dm/ios/Sources/DmPadGeometry.h`**: Posicionamiento y escalado de controles táctiles.
* **`/Users/marco/ns/dm/ios/Sources/main.m`**: Punto de entrada de la aplicación iOS y ciclo de vida de la ventana Metal.

### B. Workspace de Zelda OoT (`/Users/marco/Ocarina`)
* **`/Users/marco/Ocarina/xcode/DmIOS.xcodeproj`**: Proyecto Xcode para iOS.
* **`/Users/marco/Ocarina/xcode/Gen/`**: Archivos puente generados:
  * `embed.S`: Incluye directamente en binario el ejecutable NSO parcheado (`.incbin`).
  * `DmIOS.entitlements`: Habilita `com.apple.developer.kernel.increased-memory-limit`.
  * `resources.env`: Parámetros de entorno y rutas a RomFS y shaders.
  * `dm.xcconfig`: Variables de compilación (`DM_COPY_ROMFS = NO`).
* **`/Users/marco/Ocarina/patched_new/{main,subsdk0,sdk}.patched.bin`**: Módulos reescritos con el `patch_nso.py` del pin `88c7e03f` (sin el corte NDVE antiguo).
* **`/Users/marco/Ocarina/shader/ios/b-adec7d81d970/`**: Shaders traducidos a Metal Shading Language (MSL) para la cadena actual; 42.854 shaders, NDVE incluido.
* **`/Users/marco/Ocarina/xcode-derived-data/Build/Products/Release-iphoneos/DmIOS.app`**: Bundle final compilado.

### C. Almacenamiento en el Dispositivo (iPhone)
* **`Documents/RomFS.img` (24.44 GB)**: Sistema de archivos del juego.
  > [!CAUTION]
  > **REGLA DE ORO #1: NUNCA SOBREESCRIBIR NI BORRAR `RomFS.img` EN EL IPHONE.**
  > Pesa 24.44 GB y tarda horas en transferirse. La app ya está configurada para leer este archivo desde el directorio de documentos de la aplicación.
* **`Documents/env.txt`**: Variables de configuración de arranque del motor.

---

## 4. Comandos de Compilación e Instalación

### Preparar el proyecto Xcode (perfil y shaders del pin nuevo)
```bash
cd /Users/marco/ns
GAME=0400486024DEC000 W=/Users/marco/Ocarina \
PATCHED=/Users/marco/Ocarina/patched_new \
ROMFS=/Users/marco/Ocarina/extract/v131072/RomFS.img \
MSL_IOS=/Users/marco/Ocarina/shader/ios \
DM_COPY_ROMFS=NO DM_BUNDLE_ID=com.marcoastorga.zeldaoot \
DM_TEAM_ID=TGU3UHC7C7 DM_DISPLAY_NAME="Zelda Ocarina of Time" \
DM_APP_VERSION=1.1.1 DM_APP_CATEGORY=public.app-category.adventure-games \
DM_SHOW_FPS=1 DM_LANG=en-US \
DOTNET_ROOT="$HOME/.dotnet" dm/ios/tools/build_ipa.sh --prepare-xcode
```

### Compilar la App en Release
```bash
xcodebuild -project /Users/marco/Ocarina/xcode/DmIOS.xcodeproj \
  -scheme "Zelda Ocarina of Time (0400486024DEC000)" \
  -configuration Release \
  -destination 'generic/platform=iOS' \
  -allowProvisioningUpdates build
```

### Instalar la App en el iPhone Dams
```bash
xcrun devicectl device install app \
  --device 00008150-00190C880E3B401C \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app
```
> [!CAUTION]
> **Instalar SIN desinstalar.** La app se actualiza sobre la misma y conserva `Documents/RomFS.img` y `Documents/save`. Si se borra la app, iOS elimina el contenedor y habría que volver a copiar el RomFS de 24,4 GB.

### Probar en Mac (ventana)
El host Mac ya está compilado en `/Users/marco/ns/dm/host/dmhost`. El lanzador listo para usar es:
```bash
/Users/marco/Ocarina/run_mac.command
```
Abre una ventana con el juego; usa `save_mac` (copia de la partida) para no tocar la del iPhone, el RomFS externo y la biblioteca iOS `b-adec7d81d970`. El título se verificó a frame 600; “Continue” carga la partida.

### Configuración recomendada de `env.txt`
`Documents/env.txt` puede contener:
```ini
DM_OPMODE=0
DM_SHOW_FPS=1
DM_LAUNCHER=0
```
* **`DM_OPMODE=0`**: Handheld (720p). Ya es el valor por defecto de la app y del perfil (`op_mode handheld`); `DM_OPMODE=1` sigue permitiendo Docked.
* **`DM_SHOW_FPS=1`**: Muestra el contador de FPS.
* **`DM_LAUNCHER=0`**: Arranca directamente el juego sin pasar por el selector inicial.
* **`DM_GUEST_FILE=1`**: lo fuerza `main.m` en iOS para que la memoria invitada quede fuera de `phys_footprint`; no hace falta escribirlo.
* **`DM_TOUCH_SIZE`**, **`DM_TOUCH_STICK`** y **`DM_TOUCH_LAYOUT`**: tamaño y disposición de los controles táctiles.

---

## 5. Arquitectura de Memoria y Lecciones Críticas Aprendidas

### A. Memoria actual: el crash de ~17 s / 20 GB quedó atrás
* **Síntoma anterior:** el juego se cerraba a los ~16-17 s (frame ~798) o al pasar al gameplay (frame ~20.231), y en Mac el proceso llegaba a ~20 GB. La causa era la suma de: staging blocks de 32 MB, pools grandes sin chunking, texturas bindless subidas sin filtrar y, sobre todo, memoria invitada anónima que Jetsam cuenta como `phys_footprint`.
* **Estado actual (pin `88c7e03f`):**
  - `mem_map_limit on` en el perfil: los mapas que exceden el pool fallan como en Horizon y el `StandardAllocator` devuelve las páginas que retenía.
  - El host nuevo reutiliza tablas de pools, recicla la caché de readback, mapea el pipeline archive y solo sube texturas activas con vista.
  - `DM_GUEST_FILE=1` en iOS: la memoria invitada vive en archivos sparse, desenlazados y mapeados shared; sus páginas **no cuentan** en `phys_footprint` y el kernel puede escribirlas a disco. La app lo fuerza en `main.m`; `DM_GUEST_FILE_DIR` apunta a `NSTemporaryDirectory()`. Si no hay disco suficiente (`DM_GUEST_FILE_MIN_FREE`, 5 GB por defecto) vuelve a memoria anónima con una línea `[RED]`.
  - Cadena de shaders `adec7d81d970`: 42.854 shaders, incluido el pack NDVE de las películas.
* **Medición en Mac (iPhone 17 Pro Max como objetivo, perfil handheld):**
  - Arranque/título: **3,5 GB residentes**, GPU ~2,5 GB.
  - Tras cargar la partida: **7,4 GB residentes**, GPU ~4,0 GB.
  - Con `DM_GUEST_FILE=1`: **544 MB de `phys_footprint`** y ~3,3 GB de RSS; la memoria del juego está en archivos, fuera de la cuenta de Jetsam.
  - Ya no se alcanzan los 20 GB ni el SIGKILL de 8 GB del build anterior.
* **En el iPhone:** no hay que tocar `APP_POOL`; el perfil mantiene `0x240000000` (9 GB) y el entitlement `com.apple.developer.kernel.increased-memory-limit`. La app debe instalarse **sobre** la existente, sin desinstalar, para conservar `Documents/RomFS.img` y `Documents/save`.

---

### B. La Trampa de `APP_POOL` (¡ADVERTENCIA CRÍTICA!)
* **Error intentado previamente:** Para reducir el consumo de memoria se intentó limitar `APP_POOL` de 9.0 GB a 6.0 GB (`DM_APP_POOL=0x180000000`).
* **Consecuencia:** El juego crasheó al instante en boot con `CRASH sig=11 addr=0 pc=main+0x31bd480`.
* **Motivo:** El `StandardAllocator` del ejecutable Switch 2 (`main+0x32137a0`) calcula su heap restando el tamaño del ejecutable a `APP_POOL`. Si se reduce, el juego agota su memoria interna y las funciones devuelven punteros `NULL`, provocando un fallo de segmentación instantáneo.
* **Regla:** **`APP_POOL` DEBE PERMANECER EN `0x240000000` (9.0 GB)**. En iOS/Mach, reservar memoria virtual no consume memoria física (RAM) hasta que las páginas son tocadas.

---

### C. La Apariencia "Súper Oscura"
* **Reporte del usuario:** "Aparte se ve súper oscuro".
* **Explicación técnica:** La partida precargada (`slot_00`) inicia a Link dentro de su casa en el árbol (*Treehouse*). El interior es un tronco hueco de corteza oscura cuya única luz directa es un rayo que entra por el tragaluz y la linterna de techo.
* **Comportamiento en juego:** Al mover a Link con el joystick analógico virtual hacia la salida y presionar **A** para abrir la puerta hacia el balcón, la iluminación cambia inmediatamente a plena luz solar exterior, brillante y con rayos crepusculares renderizados a la perfección.

---

## 6. Controles y Jugabilidad en iOS

* Los controles virtuales están definidos en `dm/ios/Sources/DmGamepad.m`.
* **Joystick izquierdo:** Mueve a Link (caminar / correr).
* **Zona derecha de la pantalla (tercio derecho sin botones):** Control de cámara / stick derecho.
* **Botón A:** Interactuar, abrir puertas, subir/bajar escaleras, hablar.
* **Botón B:** Ataque / acción secundaria.
* **Botón de engranaje (arriba al centro):** Abre el menú in-game (`GameMenu.swift`) para ajustar tamaño de controles, cambiar opmode (Handheld/Docked) o salir.
* **Ajustes táctiles nuevos:** `DM_TOUCH_SIZE=small|medium|large`, `DM_TOUCH_STICK=corner|higher|inward` y `DM_TOUCH_LAYOUT` (disposición por jugador). El tamaño que no cabe retrocede hasta el que sí cabe; ningún control se sale de la pantalla ni se solapa.
* **Salir al launcher:** mantener Home del mando 2 s o usar Quit en el menú in-game; el prompt táctil permite Quit o Cancel (`quit_prompt.h`, `front_ios.m`).

---

## 7. Verificación de Logs en Tiempo Real

Para capturar la salida de consola de la app desde el iPhone:
```bash
xcrun devicectl device process launch \
  --device 00008150-00190C880E3B401C \
  --console \
  com.marcoastorga.zeldaoot
```
O inspeccionar el archivo de registro generado en la app:
```bash
cat /tmp/iphone_test_log.txt | tail -n 50
```
