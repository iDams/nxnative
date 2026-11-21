# HANDOFF TÉCNICO: The Legend of Zelda: Ocarina of Time (Switch 2 en iOS)

> **Reglas obligatorias del proyecto:** [`REGLAS.md`](REGLAS.md) — icono (`/Users/marco/Ocarina/assets/icon.jpeg`), limpieza y espacio (>20 GB libres, nunca duplicar el RomFS), instalación encima de la app (`DM_COPY_ROMFS=NO`) y cadena de shaders fija (`39417042cfe8`).

> **Documentación de sesiones previas:** [`DOCUMENTACION_SESION_2026-11-08.md`](DOCUMENTACION_SESION_2026-11-08.md).

---

> [!IMPORTANT]
> **VERSIÓN ESTABLE ACTUAL (`v1.2-oot-wideview-stable` — 2026-11-13):**
> * **Tag Git en `/Users/marco/ns` (`origin`):** `v1.2-oot-wideview-stable` (`main` y rama `stable/oot-wideview`; hitos anteriores intactos en `v1.1-oot-wideview-stable` y respaldo `16:9` `v1.0-oot-30fps-stable` / `stable/oot-30fps` `d0a4b0e3`).
> * **Cadena de Shaders:** `39417042cfe8` (`/Users/marco/Ocarina/shader/ios/b-39417042cfe8.img`, 5.4 GB, 42,848 shaders, `inputs 4bd48f723876`).
> * **Modo de Pantalla y Ajuste (`iPhone Dams`):** Hor+ Wide View `21:9` (`2352x1080`, `DM_WIDE_VIEW=1` activo por defecto) + `Documents/env.txt` limpio (`DM_DYNAMIC_RESOLUTION=1`, `DM_UPSCALER=metalfx`, `DM_THERMAL_CAP=0`, `DM_QOS_MAP=1`), con **`DM_FPS_TARGET` (default `60` en `game.tsv`) y `DM_FRAMEGEN` (default `off`) desbloqueados** en el Launcher para probar `60 FPS` o fijar `30 FPS`.
> * **Rendimiento Medido (`iPhone Dams`):** En ajuste `30 FPS` (`DM_FRAMEGEN=0`): **30.0 FPS estables clavados** (`0` cuadros caídos/descartados en `1,200` frames consecutivos `f1201–f2400`, `frame time p50 = 33.33–33.39 ms`, `p99 = 33.34 ms`, consumo CPU `0.37–1.30 W`, `wakeups 685–840/s`, `thermal 0`, `brightness 0.55`).
> * **Controles Físicos (`DmGamepad.m`):** Al conectar cualquier control físico (`Xbox`, `PlayStation`, etc.), el botón de ajustes (engrane superior central) se oculta junto al gamepad virtual y reaparece `4 s` al tocar la pantalla.
> * **Estado Gráfico:** Hierba, follaje, sombras, geometría 3D en `21:9`, menús a pantalla completa y minimapa/HUD centrados completamente estables.
> * **Cómo volver a una versión estable si un experimento futuro falla:** Ver [Sección 8 (Restauración Rápida a la Versión Estable)](#8-restauración-rápida-a-la-versión-estable-rollback).

> [!NOTE]
> **RAMA EXPERIMENTAL EN CURSO (`exp/upstream-perf-sync` — 2026-11-20):** Sincronizados los lotes `1–5C` (`f37d41ed`, `2524558d`, `5fed070a`) y la nueva ola `6A–6C` (`2cfdad9a5`, `05ac34a26`, `e4ca997ea`, `39e386eb3`, `6e198fcc3`, `17a001193`, `2e09fab66`) desde `cheemu/L`, `seedds/main` y `nx2apple/main`, preservando **`DMSH_CHAIN = "39417042cfe8"`** (`inputs 4bd48f723876`), `DM_WIDE_VIEW=1` y el auto-ocultado del engrane en `DmGamepad.m`, con `patched_37f` regenerado, suites verificadas en macOS/Simulator, y build Release compilada e instalada encima en `iPhone Dams` (`com.marcoastorga.zeldaoot`, `DM_COPY_ROMFS=NO`). Ver [Sección 16](#16-sincronización-experimental-de-rendimiento-expupstream-perf-sync-2026-11-14).

---

## 1. Repositorios Base, Referencias Comunitarias y Puntos de Restauración

| Repositorio / Fuente | URL Remota | Rama / Tag Estable | Rol |
| :--- | :--- | :--- | :--- |
| **nxnative (Fork Privado)** | `https://github.com/iDams/nxnative.git` | `main` / Tag: **`v1.2-oot-wideview-stable`** / Rama: **`stable/oot-wideview`** (fallback `16:9`: `v1.0-oot-30fps-stable` / `stable/oot-30fps`) | Motor nativo de recompilación para Switch / Switch 2 en Apple Silicon / iOS. Clonado local en `/Users/marco/ns`. |
| **nxnative (Upstream)** | `https://github.com/seedds/nxnative.git` | `main` (incluye [PR #1852](https://github.com/seedds/nxnative/pull/1852)) | Repositorio upstream original del motor `dm` (revisar PRs de rendimiento, Metal 4 y `0400486024DEC000`). |
| **kid-with-sword-remake (cheemu21)** | `https://github.com/cheemu21/kid-with-sword-remake/releases` | Releases (`v22+`) | Builds y parches comunitarios específicos para *Zelda: Ocarina of Time* (Switch 2). Revisar periódicamente sus notas de release y commits embebidos para nuevas optimizaciones de OoT. |
| **NX2Apple (Frontend & Perfiles)** | `https://github.com/LNPANDY/NX2Apple` | `main` | Frontend comunitario y parches iOS sobre `nxnative`. |
| **GreenKid-MV (adi0900)** | `https://github.com/adi0900/GreenKid-MV` | Remoto `greenkid` | Fork comunitario con optimizaciones ARM64/NVN/render para OoT y Switch 2. |
| **Entorno Local del Motor** | Ruta: `/Users/marco/ns` | Local (`main`) | Código fuente del motor (`dm`). |
| **Entorno del Proyecto Zelda**| Ruta: `/Users/marco/Ocarina` | Local | Workspace con proyecto Xcode, binarios parcheados, shaders y scripts. |

---

## 2. Información del Juego y Plataforma Objetivo

* **Juego:** *The Legend of Zelda: Ocarina of Time* (Nintendo Switch 2 / Ounce).
* **Title ID / Game ID:** `0400486024DEC000`.
* **Versión:** 1.1.1 (`v131072`).
* **SDK Nintendo:** `nnSdk-Generic_22_2_8-Release` (Major 22).
* **Módulos:** `main`, `subsdk0`, `sdk` (`/Users/marco/Ocarina/patched_37f`).
* **Dispositivo Objetivo:** iPhone 17 Pro Max ("iPhone Dams").
  * **Identificador (UDID):** `00008150-00190C880E3B401C`.
  * **Modelo de Hardware:** `iPhone18,2` (Apple A19 Pro GPU, 12 GB LPDDR5X RAM).
* **Bundle Identifier iOS:** `com.marcoastorga.zeldaoot`.
* **Equipo / Firma:** `Apple Development: Marco Astorga (L4J24857AX)` (`TEAM_ID = TGU3UHC7C7`).

---

## 3. Los 3 Pilares Técnicos de la Versión Estable (`v1.0-oot-30fps-stable`)

1. **Pipeline Puro de Mesh Shaders y Particionado Indirecto en 8 Partes (`f58c83ac`):**
   * En `dm/gfx/nvn/nvn_geometry.inc`, los mesh shaders sin task shader (`ts == nil` en la cadena `39417042cfe8`) usan `MTL4MeshRenderPipelineDescriptor` sin `objectFunction` ficticia, logrando que los **131 pipelines de task/mesh carguen desde el archivo (`0 prewarm failed`)** y que ningún draw de follaje se descarte.
   * En `dm/gfx/msl/mdx.metal` y `mtk_encode`, los draws indirectos de mesh sin task shader se dividen en `MDX_MESH_PARTS = 8` partes de hasta `1024` workgroups cada una (registro de 192 bytes en `mdx_mask`), evitando page faults de GPU cuando la hierba supera los 1,024 workgroups (hasta 1,771 en Kokiri).
2. **Estabilidad de Frame Pacing a 30 FPS y Control Térmico (`5dc2c86a` — Merge de PR #1852):**
   * Integra el nuevo controlador de cadencia de cuadros (`present_snap on`, `link_latency30 2`, `fps_payback off`, `fps_hold off` en `dm/games/0400486024DEC000/game.tsv`).
   * Elimina la deuda de vsync (`fps_payback`) y la retención artificial de cuadros (`fps_hold`) que causaban oscilaciones entre 20 y 30 FPS.
   * Con `DM_FPS=30`, `DM_FPS_TARGET=30`, `DM_DYNAMIC_RESOLUTION=1`, `DM_UPSCALER=metalfx`, `DM_THERMAL_CAP=0`, `DM_QOS_MAP=1` y `DM_QOS_EDGES=45,59`, mantiene **33.33 ms p50 / 33.91 ms p95** con la CPU a solo **2.1 W**.
3. **Corrección del Binding `first_gpu` (`+4`) en Task Shaders (`a9dc57b8`):**
   * **Causa raíz del parpadeo de hierba/objetos:** Los Task Shaders (`ts`) compilados en `b-39417042cfe8.img` leen `dm_first[0]` desde el slot 29 (`DMSH_MESH_DRAW_SLOT`) esperando el índice `first` (`+4` dentro del registro indirecto `{count, first}`). Al pasarles `+0` (`ind_rec`, donde reside `count`), cada Task Shader sumaba `count + tg` y leía instancias fuera de rango cada vez que el culling cambiaba con la cámara.
   * **Solución:** En `mtk_encode` (`dm/gfx/nvn/nvn_geometry.inc`), cuando `ts != nil`, se enlaza `first_gpu = g_mtk.ind_rec + 4` (o `&g_mtk.first` en draws directos) en `g_ot` y `enc_r_vbuf` en el slot `DMSH_MESH_DRAW_SLOT`. Además, `mdx_run` (`dm/gfx/nvn/nvn_mtl.m`) registra los draws indirectos de mesh (`words == 0`) con tamaño de 8 bytes (`{count, first}`) para detectar cualquier `OP_UPDATE_UB` sobre `first`.

---

## 4. Rutas Absolutas Críticas en el Sistema

### A. Repositorio del Motor (`/Users/marco/ns`)
* **`/Users/marco/ns/dm/gfx/nvn/nvn_mtl.m`**: Backend gráfico Metal 4 (`mdx_run`, `fstep`, pools, texturas, swap chain).
* **`/Users/marco/ns/dm/gfx/nvn/nvn_geometry.inc`**: Construcción y codificación de pipelines de Task/Mesh (`mtk_build`, `mtk_encode`, `mesh_tasks_draw`).
* **`/Users/marco/ns/dm/gfx/msl/mdx.metal`**: Compute kernel `mdx_mask` para copiado y particionado en GPU de indirect draws.
* **`/Users/marco/ns/dm/gfx/nvn/dmsh_chain.h`**: Fijado en `DMSH_CHAIN "39417042cfe8"`.
* **`/Users/marco/ns/dm/games/0400486024DEC000/game.tsv`**: Perfil del juego (`present_snap on`, `link_latency30 2`, `fps_payback off`, `fps_hold off`).

### B. Workspace de Zelda OoT (`/Users/marco/Ocarina`)
* **`/Users/marco/Ocarina/assets/icon.jpeg`**: Icono oficial obligatorio del proyecto.
* **`/Users/marco/Ocarina/shader/ios/b-39417042cfe8.img`**: Biblioteca monolítica iOS completa (5.4 GB, 42,848 shaders, `inputs 4bd48f723876`).
* **`/Users/marco/Ocarina/patched_37f/{main,subsdk0,sdk}.patched.bin`**: Binarios NSO parcheados actuales.
* **`/Users/marco/Ocarina/xcode/DmIOS.xcodeproj`**: Proyecto Xcode configurado apuntando a `/Users/marco/ns` y `b-39417042cfe8.img`.
* **`/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app`**: Bundle compilado y firmado.

### C. Almacenamiento en el Dispositivo (iPhone Dams `00008150-00190C880E3B401C`)
* **`Documents/RomFS.img` (24.44 GB)**: Sistema de archivos del juego. **NUNCA BORRAR NI SOBREESCRIBIR.**
* **`Documents/save/`**: Partidas guardadas del usuario. **NUNCA BORRAR.**
* **`Documents/env.txt`**: Variables fijas limpias (`DM_DYNAMIC_RESOLUTION=1`, `DM_UPSCALER=metalfx`, `DM_THERMAL_CAP=0`, `DM_QOS_MAP=1`); `DM_FPS_TARGET` y `DM_FRAMEGEN` quedan desbloqueados para elegirlos en el Launcher.

---

## 5. Configuración Activa de `Documents/env.txt` en el iPhone

```ini
DM_DYNAMIC_RESOLUTION=1
DM_UPSCALER=metalfx
DM_THERMAL_CAP=0
DM_QOS_MAP=1
```

> [!NOTE]
> * **`DM_FPS_TARGET` y `DM_FRAMEGEN` no se fijan aquí** para que sigan desbloqueados en la interfaz del Launcher (`game.tsv` deja `fps_target 60` y `framegen` apagado por defecto). Si se busca estabilidad total `0` caídas en `21:9`, seleccionar `30` FPS en el Launcher (`DM_FRAMEGEN` apagado).
> * **Variables retiradas por redundantes:** `DM_FRAMEGEN_MV=1` (`nvn_mtl.m` ya la activa por defecto si `DM_FRAMEGEN=1`) y `DM_QOS_EDGES=45,59` (`hle_thread.c` ya usa `44,59` por defecto, cubriendo prioridad `44` de OoT).
> * **`DM_THERMAL_CAP=0`** desactiva el recorte automático a `30 FPS` en estado térmico `>= 2` (`hle_nvn.c`); retirarla si se prefiere protección térmica automática en sesiones largas a `60 FPS`.
> * **`DM_WIDE_VIEW=1`** queda activo por defecto desde el launcher (`dm/games/settings.tsv`); si se añade `DM_LAUNCHER=0` en `Documents/env.txt`, incluir también `DM_WIDE_VIEW=1` explícitamente para que `LauncherModel.applySaved()` no vuelva a `16:9` (además de `DM_OPMODE=0` y `DM_SHOW_FPS=1` si se desean fijar).

---

## 6. Comandos de Compilación, Firma e Instalación

### Compilar `DmIOS.app` en Release
```bash
xcodebuild -project /Users/marco/Ocarina/xcode/DmIOS.xcodeproj \
  -scheme "Zelda Ocarina of Time (0400486024DEC000)" \
  -configuration Release \
  -destination 'id=00008150-00190C880E3B401C' \
  -derivedDataPath /Users/marco/Ocarina/build/ios/xcode-dd \
  -allowProvisioningUpdates build
```

### Verificar Firma y Entitlements
```bash
codesign --verify --deep --strict \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app
```

### Instalar en el iPhone Dams (SIN desinstalar) y Lanzar
```bash
xcrun devicectl device install app \
  --device 00008150-00190C880E3B401C \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app

xcrun devicectl device process launch \
  --device 00008150-00190C880E3B401C \
  --terminate-existing --activate \
  com.marcoastorga.zeldaoot
```

---

## 7. Reglas Críticas de Memoria y Almacenamiento
* **`APP_POOL` debe permanecer en `0x240000000` (9.0 GB)** en `game.tsv`. Reducirlo provoca `SIGSEGV` inmediato en `StandardAllocator`.
* **`DM_GUEST_FILE=1`** mantiene la memoria invitada en archivos sparse desenlazados fuera de `phys_footprint` para evitar que Jetsam cierre la app en iOS.
* Mantener siempre **> 20 GB libres** en `/System/Volumes/Data` (`df -h /System/Volumes/Data`).

---

## 8. Restauración Rápida a la Versión Estable (Rollback)

Si en el futuro se realizan experimentos en `/Users/marco/ns` y el rendimiento o los gráficos fallan, ejecuta estos comandos exactos para volver a esta versión estable probada:

```bash
# 1. Restaurar el código del motor al tag estable actual v1.2-oot-wideview-stable
#    (o usar v1.1-oot-wideview-stable / v1.0-oot-30fps-stable para volver a hitos previos)
cd /Users/marco/ns
git checkout v1.2-oot-wideview-stable
# (O para resetear main a la versión estable: git checkout main && git reset --hard v1.2-oot-wideview-stable)

# 2. Recompilar, verificar firma, instalar encima y lanzar en el iPhone Dams
xcodebuild -project /Users/marco/Ocarina/xcode/DmIOS.xcodeproj \
  -scheme "Zelda Ocarina of Time (0400486024DEC000)" \
  -configuration Release \
  -destination 'id=00008150-00190C880E3B401C' \
  -derivedDataPath /Users/marco/Ocarina/build/ios/xcode-dd \
  -allowProvisioningUpdates build && \
codesign --verify --deep --strict \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app && \
xcrun devicectl device install app \
  --device 00008150-00190C880E3B401C \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app && \
xcrun devicectl device process launch \
  --device 00008150-00190C880E3B401C \
  --terminate-existing --activate \
  com.marcoastorga.zeldaoot
```

---

## 9. Actualización 60 FPS + MetalFX `agl_ndsr_vr` + Dispatches Overlap (2026-11-10)

Sobre la base estable `v1.0-oot-30fps-stable` (`d0a4b0e3`), la rama `main` en `fa74c4e0` incorpora las optimizaciones verificadas de GPU para **60 FPS en iPhone** sin tocar `DMSH_CHAIN` (`"39417042cfe8"`):

1. **Escalador encadenado `agl_ndsr_vr` / `agl_ndsr_vr_light` por MetalFX (`PR #1951` — `69f2c1cf`, `3666f414`):** Registra los 37 compute shaders de `agl_ndsr_vr(_light).pck` en `DM_GAME_UPSCALER_CHAIN_KEYS`. Con `DM_UPSCALER=metalfx`, `up_chain_dispatch` omite los 3 dispatches de empaquetado intermedio y reemplaza la red neuronal final por `MTL4FXSpatialScaler` en hardware (~4.4 ms → ~0.5 ms de GPU por cuadro).
2. **Solapamiento de Compute Dispatches (`dispatch_overlap on` — `73acf68c`):** Los 117 compute dispatches al inicio de cada frame en Kokiri Forest corren en paralelo cuando el juego no emite `nvnCommandBufferBarrier` entre ellos (~5.7 ms → ~2.6 ms de GPU por cuadro).
3. **Memoria estable y probada (`mem_reserve 0x320000000`, `mem_alias 0x100000000`):** Se mantiene el layout de 12.5 GB / 4 GB alias comprobado como 100% estable en el iPhone 17 Pro Max (el experimento de 17.6 GB provocaba sig=11 al inicio por saturar el espacio de direcciones en iOS).
4. **Mejoras upstream de streaming y uniformes (`62670d62`, `33163971`, `1aa019a0`, `ee0c7ba0`):** `pool_peek` sin inflar chunks de 80 MB (PR #2085), `ubc_wait` sin contención de mutex (PR #2078), marcas de lectura de texturas preservadas en streaming (PR #2022) y colores stand-in en texturas de pool (PR #2075).
5. **Configuración de `Documents/env.txt` para 60 FPS:**
   ```text
   DM_FPS_TARGET=60
   DM_DYNAMIC_RESOLUTION=1
   DM_UPSCALER=metalfx
   DM_THERMAL_CAP=0
   DM_QOS_MAP=1
   DM_QOS_EDGES=45,59
   ```

### Comando para lanzar o reinstalar
```bash
xcrun devicectl device install app \
  --device 00008150-00190C880E3B401C \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app && \
xcrun devicectl device process launch \
  --device 00008150-00190C880E3B401C \
  --terminate-existing --activate \
  com.marcoastorga.zeldaoot
```

---

## 10. Actualización MetalFX Frame Generation + Estabilidad de Texturas y Profundidad (2026-11-12)

### 10.1 MetalFX Frame Generation (`cheemu21` rama `x12`)
* **Variables en `Documents/env.txt`:**
  ```text
  DM_FPS_TARGET=30
  DM_FRAMEGEN=1
  DM_FRAMEGEN_MV=1
  DM_DYNAMIC_RESOLUTION=1
  DM_UPSCALER=metalfx
  ```
* **Tecnología:** El render interno corre a 30 FPS nativos estables, y la GPU A19 Pro calcula vectores de movimiento por hardware (`fgmv.metal`), preserva la interfaz/HUD (`fgmv_keep`) e interpola a **60 FPS** fluidos con consumo térmico bajísimo (~1.5 W) y thermal state 0.
* **Pool de Allocators de Presentación:** Ampliado a 6 allocators (`g_present_alloc4[6]`) en `nvn_mtl.m` para desacoplar completamente los frames generados de los frames renderizados.

### 10.2 Tabla Ghost LRU para Ocarina of Time (`seedds` commit `034e91a8` — Issue #2213 / PR #2389)
* **Causa Raíz:** El ring buffer fijo de 512 entradas en `g_ghost` sufría "ring thrashing" durante streaming de áreas y transiciones con menús de pausa (hasta 3.6 millones de `puts` en 300 frames y picos de 100-490 ms en el hilo de envío).
* **Solución Implementada:** Tabla LRU con umbral de inactividad (`GHOST_IDLE=16` frames, `GHOST_IDLE_SUBS=1,024` submissions) y recorte progresivo (`GHOST_TRIM=32`). Las texturas retiradas activas permanecen en memoria sin ser expulsadas mientras se usen. `POOL_IDS_MAX` establecido en 8,192.
* **Limpieza de Vistas Stale:** `ghost_del_locked` elimina inmediatamente entradas stale de sRGB/lineal de `g_ghost` evitando fugas de referencias fantasma.

### 10.3 Zero Depth Textures mediante Copias (`seedds` commit `4cdd5577` — PR #2306)
* **Causa Raíz:** En iOS/macOS, las texturas grandes de profundidad (como `12288x12288 Depth16Unorm` y `4096x4096 Depth32Float` que OoT crea en la carga de partida) provocaban error de driver de Apple `kIOGPUCommandBufferCallbackErrorOutOfMemory` al limpiarse mediante un render pass clear-only.
* **Solución Implementada:** Se inicializan mediante copias blit directas por filas desde el buffer cero compartido, eliminando el fallo de OOM en cargas de partida.

### 10.4 Estado de la Compilación
* **Binario:** `/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app` (105 MB).
* **Firma:** `Apple Development: Marco Astorga (L4J24857AX)`, Team `TGU3UHC7C7`, Profile `iOS Team Provisioning Profile: com.marcoastorga.zeldaoot`.
* **Estado:** Compilado, empaquetado, verificado y listo para instalar en cuanto el usuario llegue a casa.

---

## 11. Actualización Amiibo + Estabilidad `seedds/main` + `DM_NDVE_LITE` (`ff7`) (2026-11-13)

### 11.1 Soporte Completo de Amiibo (`cbb84973`–`b6c31f8c`, `3f7210ef`)
* **Emulación Virtual `nn::nfp` / `nn::nfc` (`dm/host/amiibo.h`, `hle_sdk16.c`, `hle_mii.c`):** Simula etiquetas NTAG215 desde `DM_AMIIBO` (`Documents/amiibo.txt` en iOS) y persiste áreas de aplicación (`application_area`) por ID en `Documents/amiibo/<ID>.area`.
* **Selector en Menú del Juego (`GameMenu.swift`, `dm/games/0400486024DEC000/amiibo.tsv`):** Añade el submenú **Amiibo** en el menú de pausa/botón superior con 35 entradas (incluyendo `Young Link - Ocarina of Time (mask)` y `Young Zelda - Ocarina of Time (mask)` más todos los drops de Zelda/Smash) y botón `None`.
* **Overlay sin límite diario (`amiibo_overlay on`, `scripts/oot_amiibo_overlay.py`, `dm/ios/tools/stage_resources.sh`):** Genera automáticamente `DmIOS.app/romfs_overlay/amiibo-nolimit/` desde `/Users/marco/Ocarina/extract/v131072/RomFS.img`, desactiva `EnableDailyLimit` en `Game/AmiiboSetting/AmiiboSetting.game__ui__AmiiboSetting.bgyml` (`1508` bytes) y duplica su entrada REST en `ResourceSizeTable.Product.111.Nin_Ounce_NVN2.rsizetable.zs` (`11672 → 23344`). `hle_fs.c` (`rom_overlay_expand`) prioriza `amiibo-nolimit/`.
* **Escáner CoreNFC (`dm/ios/Sources/AmiiboScan.swift`, `Info.plist`):** Añade `"Scan an amiibo"` y `NFCReaderUsageDescription`.
  * **Nota de firma (Personal Team):** El perfil automático `iOS Team Provisioning Profile: com.marcoastorga.zeldaoot` (`TGU3UHC7C7`) no incluye el entitlement `com.apple.developer.nfc.readersession.formats` (`TAG`), por lo que `DmIOS.app` se compiló sin ese entitlement para mantener la firma válida. En esta instalación, `"Scan an amiibo"` detecta de forma segura la ausencia del permiso y muestra `"Not available in this install"` en lugar de cerrarse. Se pueden usar figuras físicas mediante una automatización NFC de la app **Atajos (Shortcuts)** que escriba el ID de 16 dígitos hex en `Documents/amiibo.txt`, o usar directamente la lista virtual integrada en el menú.

### 11.2 Optimizaciones de GPU/CPU, Deadlines RT y Películas Ligeras
* **Motor gráfico y NVN (`seedds/main`):**
  * `pool_skip_retired on` (`3360b204`): Evita reconstruir texturas ya finalizadas en tablas de pool tras cambios de escena.
  * `ALLOC_SHARE=64`, `POST_RING` y `VIS_RING=SUB_FLIGHT_MAX` (`5d1502f3`, `7602ff26`, `49573eae`): Allocators compartidos por frame y anillo de visibilidad para envíos frecuentes sin bloquear `nvnSyncWait`.
  * `gw_commit` iterando solo páginas anotadas (`b166ba08`), media móvil en `nvn_fstep_next` (`27b7995a`), barreras blit `mfx_order` en MetalFX (`16405b41`, `421873d7`, `3b3c8979`) y notas dispersas `treg_sparse_notes` (`36197a1b`).
  * Declaración de deadlines RT (`nvn_rt.h`, `e4e8816d`, `41b3e8ef`) en hilos `replay`, `encode` y `acquire` para evitar ejecución en efficiency cores a 30 FPS.
* **Ruta ligera de vídeo (`DM_NDVE_LITE`, `cheemu/ff` `ff7` → `931f6764`):** Por defecto evita ejecutar la red neuronal de `nn::ndve` en las películas de título/introducción (`pic_copy` + `resample_lite` en `ndve_engine.metal`), eliminando la caída por debajo de 60 FPS durante el título (`DM_NDVE_LITE=0` restaura la red completa).
* **Cadena fija:** `DMSH_CHAIN="39417042cfe8"` intacta, binarios `/Users/marco/Ocarina/patched_37f` intactos y FrameGen actual X14 (`fgmv.metal`) preservado.

### 11.3 Pruebas Verificadas y Estado Actual (`ff47e0378817`)
* **Shaders iOS (`make -C dm/host GAME=0400486024DEC000 MSL_SDK=iphoneos host_msl game_profile`):** 15 librerías Metal compiladas limpiamente (`host_msl_gen.h`).
* **Pruebas unitarias (`dm/host/tests` + `dm/gfx/tests` + `test_ndve_engine.py`):**
  * `test_ndve_engine.py`: 4 tests `OK`.
  * `t_amiibo`: `ok`; `t_nfc`: 95/95 checks passed.
  * `t_pool_retired` (11), `t_sbw` (38), `t_sparse_pool_cache` (26), `t_thread_rt` (8), `t_rt_comp` (745/245/245), `t_wait_event` (100/100), `t_dlss_mfx` (18/18/18), `t_fstep` (80), `t_present_crop` (117×4): **0 fallos**.
* **Build iOS:** `/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app` (`5.5G`, firmado para `com.marcoastorga.zeldaoot`, `DMCommit=ff47e0378817`, `DMChain=39417042cfe8`), con `romfs_overlay/amiibo.tsv` y `amiibo-nolimit/` integrados y `DM_COPY_ROMFS=NO`. Espacio libre en disco: `114 Gi`.
* **Pendiente en casa:** El iPhone `00008150-00190C880E3B401C` figura como `unavailable` ahora mismo. Al conectarlo, instalar encima con `xcrun devicectl device install app` sin borrar la app para conservar `Documents/RomFS.img` y `Documents/save`.

---

## 12. Novedades `seedds/main` (PR #2367, Hilos, FS) + Proyecto Futuro `Shakoquo/Mako` (2026-11-13)

### 12.1 Parches aplicados desde `seedds/main` (Mac / iOS, cadena fija `39417042cfe8`)
* **PR #2367 (`f23056a8` ← `5b0f1372`, `dm/gfx/nvn/nvn_fstep.h`):** Al bajar de un hold más largo (20 o 30 fps tras cargar un área en OoT), `nvn_fstep_next` ahora evalúa también la media móvil (`ccur`) frente a `FSTEP_FIT * (k - 1)`, evitando quedar atascado ~24 s en 20/30 fps por cuadros aislados sobre el umbral.
* **Seguridad de hilos (`dm/host/hle_thread.c` — `8d8023b1`, `3d7d39f6`, `8ebf615a`):** Bloqueo `G` en `GetThreadId`/`GetThreadCoreMask`/`SetThreadCoreMask` (evita UAF al cerrar), espera correcta en pausa hasta retorno del manejador y liberación limpia al cerrar hilos no iniciados.
* **FS y Empaquetado (`f07b3c1c`, `2b9d89f3`):** Limpieza de rollback en `dm/host/hle_fs.c` y rechazo recursivo en `scripts/pack_nup.py`.
* **Excluidos intencionalmente:** `a4b59097` y `ad8bca6c` (cambian `DMSH_CHAIN` respecto a `39417042cfe8`), `892aaf83` (solo Vulkan/PC), `2c3fcae5` (`PR #2114`, transform feedback de otros juegos con conflictos en `nvn_mtl.m`) y `32faef7c`/`0d103787` (diagnóstico / refactor de segmentos dependiente de commits previos no presentes en esta rama).
* **Nota sobre `cheemu/ff` (`173eb379` `FF7`):** Además de `DM_NDVE_LITE` (ya integrado), `FF7` introduce `DM_WIDE_VIEW` (parches ARM64 en `patches.py`), pero requiere regenerar `patched_37f` y revierte FrameGen a `FF3`. Se mantiene nuestra base con `X14` + `DM_NDVE_LITE` + `Amiibo` sin tocar `patched_37f`.

### 12.2 Proyecto futuro guardado: `Shakoquo/Mako` (`/Users/marco/Mako`)
* **Repositorio:** `https://github.com/Shakoquo/Mako` clonado en `/Users/marco/Mako` (rama `amd-fix-sharkiix-version`, tag `mako-4.0`, commit `a5ae5ae9`, 67 MB).
* **Qué es:** Fork de `NotRexed/Hookshot` que recompila estáticamente *Ocarina of Time* Switch 2 (`0400486024DEC000`, `v131072`, build `e0f3ccfa`) de ARM64 a C para **PC (Windows x64 / Linux `x86-64-v3`)** sobre **Vulkan 1.3 + mesh shaders** (con FSR 4 / Redstone, 45,914 pipelines precompilados, *Test my PC* y `steady_fps`). **No** compila para iOS/Mac ni incluye soporte Amiibo.
* **Qué aprovechar a futuro:**
  1. **Mako Master Quest** (`nx2/mq_banc.py`, `host/master_quest.c`): remixes RomFS de Deku Tree, Dodongo's Cavern y Jabu-Jabu (hasta que Link despierta adulto), precios RSDB y multiplicador de daño configurable.
  2. **Offsets documentados de `v131072`** (`nx2/oot_patches.py`, `oot_wide.py`, `oot_occlusion.py`, `oot_native_resolution.py`): velocidad de texto, cámara sin auto-centrado, desbloqueo >60 fps y escala 3D al 100%.

### 12.3 Compilación e instalación en iPhone Dams (`00008150-00190C880E3B401C`)
* Compilado e instalado encima de la app existente (`DM_COPY_ROMFS=NO`) conservando `Documents/RomFS.img` y `Documents/save`.

---

## 13. Diagnóstico y Corrección del Cierre al Iniciar (`CRASH sig=11 addr=0x8 pc=main+0x42cb58`) (2026-11-13)

### 13.1 Causa Raíz Detectada en `log.txt`
* **Síntoma:** El juego se cerraba exactamente a los 2 segundos de boot tras montar `save.work` e inicializar el memory pool de Metal (hilo `ModuleSystem` cargando el primer `ModelResource` / `gsys`):
  `CRASH sig=11 addr=0x8 pc=main+0x42cb58 lr=main+0x436d48 sp=0x17480eb30`
* **Mecanismo:** La rutina `0x4366e0` pedía una asignación de `0x1138` bytes al heap en `sp+0x38` (`0x43690c: blr x8`), la cual devolvía `x0 = 0` (`NULL`). Seguidamente el allocator de bump `0x358e660` calculaba `align_up(0 + 0x430, 8) = 0`, almacenando puntero nulo en `x23` y llamando a `0x42cb30(0)` que ejecutaba `0x42cb58: str x1, [x0, #8]`.
* **Causa del retorno nulo:** Al habilitar `romfs_overlay/amiibo-nolimit/`, se incluyó `System/Resource/ResourceSizeTable.Product.111.Nin_Ounce_NVN2.rsizetable.zs`. En `scripts/oot_amiibo_overlay.py`, `_zcmp` comprimía por stdin con `zstd -19 -c`, produciendo una trama Zstandard con `Window Size: 8.00 MiB` y checksum `XXH64`. El descompresor nativo de Nintendo (`nn::zstd` en Horizon) rechaza ventanas mayores a 2 MiB con `ZSTD_error_frameParameter_windowTooLarge`. Al fallar la descompresión del `ResourceSizeTable`, la tabla de tamaños de recursos quedó vacía y todos los heaps de recursos fallaron al asignar.

### 13.2 Corrección (`610811a9`)
* **Archivo modificado (`scripts/oot_amiibo_overlay.py`):** `_zcmp` ahora usa un archivo temporal y los modificadores `--no-check -f`, permitiendo a `zstd` inspeccionar el tamaño real de la entrada y fijar `Window Size: 1.68 MiB (1761406 B)` con `Check: None`, idéntico a las tramas originales de Nintendo.
* **Info.plist:** Actualizado el commit hash a `0fd9936d1eb4`.
* **Compilación y Despliegue:**
  * Recompilado en Release con `xcodebuild` (`DM_COPY_ROMFS=NO`).
  * Firma verificada con `codesign --verify --deep --strict`.
  * Instalado limpiamente sobre `iPhone Dams` (`00008150-00190C880E3B401C`) sin desinstalar.
* **Verificación en el dispositivo:**
  * La aplicación arrancó y superó la fase crítica de carga de `ModuleSystem`.
  * `log.txt` confirma: `NO CRASH!`, frame generation activa, decodificación de vídeo AV1 1080p con VideoToolbox (`[codec] AV1 decoder 1`), NFC inicializado con soporte virtual amiibo (`nfp::Initialize`), y el proceso (`DmIOS`) se mantiene en ejecución activa y fluida a 30/60 FPS.

---

## 14. Versión Estable Hor+ Wide View `21:9` + Ajuste `30 FPS` (`v1.1-oot-wideview-stable`) (2026-11-13)

> [!IMPORTANT]
> Promovida desde `exp/wide-view` (`b53e81e5`, tag `v1.1-oot-wideview-exp`) hacia **`main`** y la rama estable **`stable/oot-wideview`** (tag **`v1.1-oot-wideview-stable`**) tras validar la configuración estable en `30 FPS` (`DM_FRAMEGEN=0`) en `iPhone Dams`. La rama y tag históricos `stable/oot-30fps` (`d0a4b0e3`, `v1.0-oot-30fps-stable`) se conservan intactos como punto de respaldo `16:9`.

### 14.1 Cambios implementados (`DM_WIDE_VIEW=1`, sin tocar `DMSH_CHAIN`)
* **`dm/ios/Sources/front_ios.m`:** Cuando `DM_WIDE_VIEW=1`, detecta el aspect ratio de la ventana del iPhone (`1.8 <= r <= 2.6`) y exporta `DM_WIDE_ASPECT` antes de iniciar el host (en `iPhone Dams`: `2.173:1` en `956x440` pts).
* **`dm/host/loader.c` (`dm_fps_state_apply`):** Calcula los anchos múltiplos de 16 para `DM_WIDE_VIEW=1` + `DM_WIDE_ASPECT`, los escribe en la página compartida `+0x100..+0x114` y pasa `w1 x 1440` a `mtl_present_screen` (en `iPhone Dams` `2.173:1`: salida `3136x1440`, handheld `2352x1080`, aspecto `2.1778`).
* **`dm/games/0400486024DEC000/patches.py` (`wide_view`):** Stubs ARM64 en `v131072` (`ctx.base == 0x6e6be0`) para dimensiones de render (`0x1202ee0`, `0x31f39f4`, `0x31f3a00`, `0xecbc80`, `0x112c604`), cámaras 3D/proyecciones (`0x9b5f38`, `0x2e1be40`, `0x14fc7e4`, `0xe07154`, `0x27b13f8`, `0x2e1cb84`, `0x4c638c` y 11 stores de `sead::PerspectiveProjection`), canvas 2D (`0xe8303c`) y mapeo mundo→layout para marcadores/burbujas (`0x20c2b0`, `0x20c99c`, `0x215fb0`). Registra `WIDE_VIEW` en `ctx.capabilities`.
* **`dm/gfx/nvn/hle_nvn.c` (`dm_wide_note`):** Registra en el log qué anchos de canvas 2D y aspectos de cámara tomó el juego (`+0x120..+0x13f`).
* **`dm/gfx/nvn/nvn_mtl.m` (`wide2d_on`, `wide2d_fix` y ajuste de scissor en `draw_`):**
  * Mantiene la proyección del layout 2D (`3840x2160`) centrada en su relación `16:9` dentro de la pantalla ancha (factor `f = (16/9) / viewport_aspect`, desactivable con `DM_WIDE_HUD=0`).
  * Detecta quads 2D centrados de pantalla completa en `S.ssbo[0][6]` (`ndc_hw >= 0.96`, `|ndc_cx| < 0.05`) para conservar de borde a borde los fondos/oscurecidos (como el menú *Load*).
  * Ajusta horizontalmente las tijeras (`MTLScissorRect`) de subpaneles no fullscreen en `draw_()` usando el mismo centro `g_w2d_cx` y factor `g_w2d_f`, corrigiendo el desplazamiento de scrolls y del minimapa en juego.
* **`dm/games/settings.tsv` y `docs/env_switches.tsv`:** Agrega el ajuste `DM_WIDE_VIEW` (activo por defecto en la UI) y documenta `DM_WIDE_VIEW`, `DM_WIDE_ASPECT` y `DM_WIDE_HUD`.

### 14.2 Pruebas y Verificación Visual
* **Unit tests ejecutados tras el último cambio en código:**
  * `t_present_crop`: `117 checks, 0 failed` (4 ejecuciones).
  * `t_dlss_mfx`: `18 checks, 0 failed` (3 ejecuciones).
  * `t_ub`: `47 checks, 0 failed`.
  * `python3 -m unittest dm/tools/tests/test_lint_c.py dm/tools/tests/test_patches.py dm/tools/tests/test_settings.py`: `43 tests OK`.
* **Build y dispositivo (`iPhone Dams` — `00008150-00190C880E3B401C`):**
  * Compilado en `Release` con `DM_COPY_ROMFS=NO`, firma verificada (`codesign --verify --deep --strict`), instalado sin borrar datos ni saves.
  * Confirmado visualmente en juego: vista `2352x1080` sin barras negras ni estiramiento 3D, oscurecido del menú *Load* completo de borde a borde y minimapa/paneles centrados y alineados.

### 14.3 Diagnóstico de Rendimiento y Verificación Estable (`DM_FRAMEGEN=0`)
* **Causa de la caída de FPS con `DM_FRAMEGEN=1` en `21:9`:** La interpolación de FrameGen (`2125x976` vía `FG_BUDGET`) consumía **`p50 10.749–11.542 ms`** (`p95 11.036–26.817 ms`) de GPU por frame interpolado sobre un renderizado base de `~18.4–19.1 ms` GPU, saturando el presupuesto de `33.3 ms` y provocando ráfagas de `14–15` cuadros caídos/300 (`p99 75.01–100.01 ms`) con pausas automáticas de interpolación (`#1`/`#2`).
* **Solución estable (`DM_FRAMEGEN=0` + `DM_FPS_TARGET=30` + `DM_DYNAMIC_RESOLUTION=1` + `DM_UPSCALER=metalfx` en `Documents/env.txt`):**
  * Medido en vivo en `iPhone Dams` en `21:9` (`1568x720 → 2352x1080` hasta `1046x480 → 2352x1080` con MetalFX spatial scaler HDR `0.7–1.1 ms`):
  * **4 ventanas consecutivas (`f1201–f2400`, 1,200 cuadros):** **30.0 FPS clavados**, **`dropped 0 (discarded 0)`**, `frame time p50 33.33–33.39 ms` / `p99 33.34 ms`, `CPU 0.37–1.30 W`, `wakeups 685–840/s`, `thermal 0`, `brightness 0.55` estables.
  * Validado por el usuario en dispositivo y promovido como versión estable (`v1.1-oot-wideview-stable`).

---

## 15. Auto-ocultado del Botón de Ajustes con Control Físico y Limpieza de Configs (`v1.2-oot-wideview-stable`) (2026-11-13)

* **`dm/ios/Sources/DmGamepad.m`:**
  * Cuando cualquier control físico (`GCController`) se conecta (`_padOff == YES` en `padsChanged:`), el botón superior de ajustes (`DmMenuButton *_menu`) se oculta (`fade` de `0.25 s` + `hidden = YES`) junto con `_controls`.
  * Al tocar la pantalla (`touchesBegan:`) o el propio botón mientras se desvanece (`UIControlEventTouchDown`), `_menu` reaparece (`peekMenu`) durante `4 s` (`K_MENU_PEEK_S = 4`, refrescado al soltar en `endTouches:`).
  * `DmMenuButton` mantiene `_menu` visible mientras su `UIMenu` está abierto (`willDisplayMenuForConfiguration:` / `willEndForConfiguration:`), y vuelve a mostrarse de forma permanente si se desconectan todos los controles físicos (`!any`).
* **Auditoría y limpieza de configuración en `iPhone Dams` (`Documents/env.txt`):**
  * Eliminadas `DM_FRAMEGEN_MV=1` (por defecto activada en `nvn_mtl.m`) y `DM_QOS_EDGES=45,59` (el defecto `44,59` en `hle_thread.c` ya cubre `main thread` / `BaseProcExecutor` de OoT en `44`).
  * Desbloqueadas `DM_FPS_TARGET` y `DM_FRAMEGEN` (quitadas de `env.txt`) para poder elegir `60` / `30 FPS` y FrameGen desde el Launcher (`game.tsv` deja `60 FPS` por defecto).
  * Conservadas `DM_DYNAMIC_RESOLUTION=1`, `DM_UPSCALER=metalfx`, `DM_THERMAL_CAP=0` y `DM_QOS_MAP=1` (esta última opcional para ahorro energético en hilos secundarios).

---

## 16. Sincronización Experimental de Rendimiento (`exp/upstream-perf-sync`) (2026-11-14)

> [!WARNING]
> **Rama experimental (`exp/upstream-perf-sync`) — `main` y `stable/oot-wideview` permanecen en `v1.2-oot-wideview-stable` (`02e6cdda`) hasta validar el comportamiento en dispositivo.**

### 16.1 Motivación
* En pruebas previas con la compilación `v1.2` dentro del Árbol Deku (`2868x1316`, Metal 4 HUD), se observó caída a **~22 FPS** (`GPU 36.32 ms`, `Frame Interval 45.28 ms`, `App 4.18 GB`, `Available Memory 4.41 GB`), mientras que a `30 FPS` estable (`DM_FRAMEGEN=0`) rendía sin caídas. Se sincronizaron mejoras de `seedds/main`, `cheemu/ff` (`FF10`) y `nx2apple` preservando **`DMSH_CHAIN = "39417042cfe8"`** y todas las características del fork (`DM_WIDE_VIEW`, auto-ocultado del engrane en `DmGamepad.m`, PSO asíncrono `3d0351d7`, `X14` `d182396d`, `4cdd5577`, `a9dc57b8`).

### 16.2 Cambios Integrados por Lotes
1. **Lote 1 (`edf62957`..`bc5a7427`, `1e385167`, `ff7fd0bd`):** Índice por celda para `alias_take_over` (`8f221638`), snapshots tras `clear` (`edf62957`), capa `nvn_owner.m` + caché de vistas por textura Metal (`595c4c6e`), preconversión de draws indirectos `DM_NVN_IXC_PRE` (`9b720181`), MetalFX con menos copias scratch (`e38154cc`), predicción PSO en el primer miss nuevo sin stand-ins redundantes (`e1d7c581`, `bc5a7427`), señales de eventos NVN sin cerrar render pass (`1e385167`) y `ReadTexels` fuera del hilo submit (`ff7fd0bd`).
2. **Lote 2 (`1f81f50c`):** Spares en pools, parcheo de UBO (PR #1611/#1808), `dispatch` barrier (PR #2250, reemplazando el ajuste `dispatch_overlap on` de `game.tsv`) y vistas de imagen sRGB (PR #1308).
3. **Lote 3 (`343d0638`, `31d6ff92`, `7a85d7e2`):** DisplayLink pacer follow (PR #2590), paso de cuadro GPU-inactiva/sin pacer (PR #1942), esperas de copias en snapshots (PR #2153), retiro de pools con fence abierto, serialización/cosecha de archivos PSO, foco `DM_CTL`, reclamo `hostseg` por solapamiento, invalidación `htab` inline, restauración de readback 3D `b2t` y censo VM en crash.
4. **Lote 4A (`e526ad7a`):** Sustitución de `.zs` dañados en RomFS (`hle_fs.c`, `game.py`, `game.tsv` `romfs_substitute`), snapshot de directorio overlay `O(N log N)` y `EIO` en `dir_next`.
5. **Lote 4B (`026d130d`):** Háptica DualSense/Joy-Con en Apple (`hle_hid.c`, `haptics_apple.m`), respaldo Amiibo por Shortcuts (`AmiiboScan.swift`, `GameMenu.swift`) y guarda `GD` en `dm/ios/tools/build_ipa.sh`.
6. **Lote 4C (`1d7bd31f`):** FrameGen de 3 niveles `FF10` (`dm/gfx/msl/fgmv.metal`, `hle_nvn.c`, `nvn_mtl.m`), `mtl_pool_stats`, ampliación de `dm/games/0400486024DEC000/pso_manifest.tsv` (`17,870` líneas) y corrección de regresión en `copy_t2t_enc` / `copy_t2t` (`bc_alias_copy` devuelve `nil` sin crear textura Metal y `gsb_store` solo corre cuando `d != nil`).
7. **Tests adaptados al fork (`1d7bd31f`):** `t_mesh_first_use.m` (binding `buffer(29)`), `t_pool_chunks.m` (inicialización/liberación bajo `pool_skip_retired=on`) y `check_t_upload_src` en `dm/gfx/tests/Makefile` (`DM_NVN_UP_WINDOWS=0`).

### 16.3 Verificación en macOS e Instalación en `iPhone Dams` (`1d7bd31f306f`)
* **Tests verificados tras el último cambio de código (`1d7bd31f`):**
  * `make -C /Users/marco/ns/dm/gfx/tests -j8 GAME=0400486024DEC000 run`: salida `0` (`t_bc_alias` 19/0, `t_cpu_storage` 99/0, `t_mesh_first_use` 53/0, `t_upload_src` 41/0, `t_zskip` 38/0, `t_zsnap`, `check_t_pool_chunks`, `test_dmsh_pull.py` 4 tests OK, `test_tbdr_budget.py` 5 tests OK).
  * `dm/host/tests` (`t_fs` 156/0, `t_fs_directory` 35/0, `t_hid` 205/0, `t_pad_hid` 27/0, `t_motion` 209/0, `t_haptics`, `t_haptics_apple`, `t_motion_apple`, `t_pad_apple`, `t_amiibo`): todos `PASS`.
  * `test_game_profile.py` (4 tests OK), `swiftc -parse dm/ios/Sources/*.swift` OK, `dm/ios/tests` (54 tests OK). (`test_docs.py` conserva 2 fallos preexistentes en `docs/agents/game-patches.md` y READMEs faltantes de juegos eliminados).
* **Compilación e instalación:** `/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app` (`DMCommit=1d7bd31f306f`, `DMChain=39417042cfe8`, `DM_COPY_ROMFS=NO`, `CODESIGN OK`), instalada encima en `iPhone Dams` (`bundleID: com.marcoastorga.zeldaoot`). Lanzamiento remoto pendiente por bloqueo del teléfono (`Locked`).

### 16.4 Sincronización Lotes 5A–5C (`f37d41ed`, `2524558d`, `5fed070a` — 2026-11-15)
* **Lote 5A (`f37d41ed`):** Constructor/slot-pinning de páginas sucias (`dm/gfx/nvn/nvn_dirty.c`, `t_dirty_init.c`, `t_dirty_reuse.c`), wake sin lock y planificación/eventos (`dm/host/hle_svc.c`, `dm/host/hle_thread.c`), rejilla táctil de `4 ms` (`dm/host/hle_touch.c`), vaciado del archivo PSO en lifecycle (`dm/host/hle_lifecycle.c`, `t_pso_archive.m`, `test_pso_archive.py`), `guest_host_margin` y notas actualizadas en `dm/games/0400486024DEC000/README.md`.
* **Lote 5B (`2524558d`):** Agrupación de colas NVN (`dm/gfx/nvn/nvn_queue.inc`), drenaje en present (`dm/gfx/nvn/nvn_link_drain.h`), LRU de reciclaje de render targets (`dm/gfx/nvn/nvn_rt_recycle.h`), tiled resolves / Metal 4 / DLSS (`nvn_mtl.m`, `nvn_owner.m`, `hle_nvn.c`, `nvn_rec.h`), corrección de margen en `nvn_fstep.h`, más restauración/ajuste de pruebas (`DM_PRESENT_SNAP=0` en `check_t_present_rescue` de `dm/gfx/tests/Makefile` y regla `$(B)/t_input_stream` en `dm/host/tests/Makefile`).
* **Lote 5C (`5fed070a` — `cheemu/ff` FF11 + `nx2apple`):**
  * **Cool Mode y Render Scale (`dm/host/dm.h`, `dm/host/loader.c`, `dm/games/settings.py`, `dm/games/settings.tsv`, `dm/games/0400486024DEC000/game.tsv`):** `DM_COOL=1` fuerza `DM_RENDER_SCALE=0.5` y `DM_FRAMEGEN_SCALE=0.5` en `dm_fps_state_apply`; validación `0.25f <= sc && sc <= hi`; en `game.tsv` se agregan `render_scale 0.25:1.0:0.05`, `render_scale_marks 0.25=Potato` y `render_base 2599x1462 1949x1097` (conservando `fps_target 60`, `op_mode handheld`, `romfs_substitute`, `amiibo_overlay`, `gpu_timestamps`, `link_latency30`, `fps_payback`, `present_snap`).
  * **Pausa automática de FrameGen en menú (`dm/gfx/nvn/nvn_mtl.m`):** `fg_menu_pause()` (`DM_FRAMEGEN_MENU_PAUSE`: pausa por debajo de `300` draws durante `6` presents tras armarse a los `900` frames y reanuda tras `2` presents), bandera `dm.framegen.unsupported` en `fg_on`, y logging de los primeros `8` errores en `q_cb` (`addCompletedHandler`).
  * **Nuevo Launcher iOS (`LauncherHome.swift`, `Launcher.swift`, `LauncherRow.swift`, `LauncherSettings.swift`, `GameMenu.swift`, `build_ipa.sh`):** Pantalla inicial con dial `HeatDial`, tiles rápidos `QuickTile`, cajón `AllSettingsDrawer` con pestaña `Look` (`LauncherBackgroundStore`) y soporte `DM_LAUNCHER_BG`/`DM_LAUNCHER_TITLE`; defaults iOS en `settings.tsv` a `DM_FPS_TARGET=30` y `DM_RENDER_SCALE=0.6`, además de `deviceSetup()` (`DM_COOL=1` bajo `< 10 GB` RAM). Se mantienen intactos `AmiiboScan`, `dm_quit_menu()`, `DmGamepad.m` y `main.m`, sin importar `SaveSyncReport` ni `DM_PAD_THEME`.
  * **Audio Mix (`dm/ios/Sources/audio_session_policy.h`, `front_ios.m`):** Soporte en vivo para `DM_AUDIO_MIX` mediante `dm_audio_session_options` / `dm_audio_session_apply` en `front_ios_audio_session`, preservando `dm_mic_wanted()`.
* **Verificación en macOS tras los ajustes de test (`5fed070a`):**
  * Actualizados `dm/ios/tests/launcher_storage_check.swift` (para el default iOS `30 fps` / `0.6`) y `dm/tools/tests/test_dmsh_abi.py` (`KINDS` excluye `"?"` de logs).
  * `xcrun --sdk iphoneos swiftc -typecheck ... DmIOS/Sources/*.swift`: `0` errores.
  * `make -C dm/gfx/tests -j8 GAME=0400486024DEC000 run`: salida `0`.
  * `make -C dm/host/tests -j8 GAME=0400486024DEC000 run`: salida `0`.
  * Suite Python (`13` módulos de iOS, settings, launcher, ABI, `env_switches`, C lint y arquitectura): `89` tests `OK`.
  * `test_docs.py Length GameNotes`: `OK` (`Links` conserva `13` referencias previas de upstream a juegos/secciones ausentes en este fork).

### 16.5 Ola de Sincronización `6A–6C` (`2cfdad9a5`..`2e09fab66` — 2026-11-20)
* **Lote `6A` (`2cfdad9a5` — host, audio, micrófono y threads):** Parches de audio/playback, sensibilidad de micrófono (`DM_MIC_BOOST` + `mic_level.h`), cambios en `hle_thread.c`/`hle_svc.c`/`hle_audio.c` y decodificación de vídeo (`hle_movie_avc.c`).
* **Lote `6B-1` (`05ac34a26` — `cheemu/L` `L6–L7.2` + memoria/perfil OoT):**
  * `dm/games/0400486024DEC000/game.tsv`: región alias de `8 GB` (`mem_alias 0x200000000`, `mem_reserve 0x420000000`) y `mem_map_limit off` para evitar pantalla blanca tras la Master Sword en iOS.
  * `dm/games/0400486024DEC000/patches.py`: umbral de resolución dinámica adaptado en host a `30 fps` (`state +0x2a4`) y límite superior de `DM_RENDER_SCALE` (`render_scale`), manteniendo el corte `CUTS` de NDVE y `wide_view`.
  * Texturas/rendimiento NVN y host: `DM_NVN_TEX_MIP_SKIP` (`512/0`, activo por defecto en iPhone en `dm/games/settings.tsv`), límite de upload `48 MB`, compilación de PSO sin bloqueo (`pso_nw_get`), presupuestos de hilos RT en iOS (`rt_share` `0.4` acquire / `0.55` frame), pacing `tu_tick()` (`TU_MAX_DEF=32`, `TU_EVERY_DEF=2`) y `svc_thread_dump_live` débil en `hle_nvn.c`.
* **Lote `6B-2a` (`e4ca997ea`):** Compartición/recorte de arena NVN (`DM_NVN_ARENA_SHARE`, `DM_NVN_ARENA_TRIM`), ventana ociosa de upload, replay de logs de pool, bandas cero, holgura RT y correcciones de colas/presentación.
* **Lote `6B-2b-1` & `6B-2b-2` (`39e386eb3`, `6e198fcc3`):** Conformidad y caché de alias de texturas, lookups/demanda de PSO en segundo plano, alpha/rango DLSS, latch de display link, archivo PSO, tiles dispersos, race de páginas sucias y upload/feedback/geometría (`nvn_feedback.inc`, `nvn_ndve.inc`, `nvn_geometry.inc`, `nvn_dirty.*`, `nvn_mtl.m`). En `pso_desc` de `nvn_mtl.m`, la comprobación `ONE + ZERO` en floats se evalúa antes de `blend_zero_src1` para no desactivar blending en fuentes secundarias faltantes (`t_missing_src1`).
* **Lote `6C` (`17a001193`, `2e09fab66` — `L6.2–L6.6`, HUD, Joy-Con individual y Mods iOS):**
  * **UI & HUD (`Launcher.swift`, `LauncherHome.swift`, `main.m`, `front_ios.m`, `nvn_mtl.m`):** Estilo oscuro forzado en `DmHostingController` (`overrideUserInterfaceStyle = .dark` + `.environment(\.colorScheme, .dark)`), default `DM_RENDER_SCALE=0.50` junto a `DM_COOL=1` bajo `10 GB` de RAM, interruptor `DM_LINK_30HZ` en `link_30hz()`, aplicación temprana de `launcher.DM_METAL_HUD` en `boot_env()` antes de `UIApplicationMain` (`front_ios_hud_enabled_from_saved_choice()`) + `front_hud_apply()` en `startGame`/`viewDidLayoutSubviews`. Se mantienen todos los selectores iOS en `dm/games/settings.tsv`, `DM_OPMODE=0`/`DM_GUEST_FILE=1` en `main.m`, y `AmiiboScan`/`dm_quit_menu()` en `GameMenu.swift`.
  * **Joy-Con individual en iOS (`DmGamepad.m`):** Lectura vía `GCControllerLiveInput` (`pad_live_input`/`pad_usable`/`pad_listen`), preservando el auto-ocultado del engrane de ajustes (`K_MENU_PEEK_S = 4`), `front_ios_quit_poll()` y `front_ios_quit_controllers()`. Añadidos tests `testLiveJoyConMapsSidewaysAndKeepsItsOwnPlayer` y `testLiveJoyConsJoinOnButtonAndKeepPromptInputOutOfGame` en `dm/ios/tests/duo/Tests/DuoHarnessTests.m` (con `AmiiboScan.swift` y stubs en `DuoRuntimeStubs.m`/`project.yml`).
  * **Gestor de Mods iOS (`dm/host/mods.h`, `mods_zip.h`, `mods_api.h`, `hle_mods.c`, `hle_fs.c`, `dm/ios/Sources/Mods.swift`, `scripts/oot_mods_rstb.py`, `dm/ios/tools/build_ipa.sh`, `stage_resources.sh`):** Importación `.zip`/carpeta hacia `Documents/mods/<nombre>/romfs`, orden de prioridad en `DM_MODS`, fusión de tabla `RESTBL` (`DM_MODS_RSTB`/`DM_MODS_WORK`) y enlace con `-lcompression` en `dm/host/Makefile` y `dm/ios/tools/gen_build.py`. En `2e09fab66`, se agregó el empaquetado de `mods_rstb.bin` (`1,761,481` bytes) en `stage_resources.sh` y fallback a zstd de Homebrew en `oot_mods_rstb.py`.
* **Regeneración de `/Users/marco/Ocarina/patched_37f/`:** Reconstruidos `main`, `subsdk0` y `sdk` con `patch_nso.py` (`main.fixups.tsv`: `RENDER_SCALE=1`, `DYNAMIC_RESOLUTION=1`, `WIDE_VIEW=1`, `BUILD_ID=e0f3ccfaf50065ae4ea01eb5aeb13b1a`, `cuts 33`).
* **Verificación en macOS / Simulador:**
  * `DMSH_CHAIN`: `"39417042cfe8"` intacto (`dmsh_chain.py --inputs`: `4bd48f723876`).
  * `make -C dm/gfx/tests GAME=0400486024DEC000 -j4 run`: salida `0` (`test_dmsh_pull.py` 4 tests `OK`, `test_tbdr_budget.py` 5 tests `OK`).
  * `dm/host/tests`: `t_mods` (`87 checks, 0 failed`), `t_fs` (`PASS`), resto de pruebas (`t_language`..`t_symbols`, `t_preempt --all-peers`, `t_pause_stress`) `PASS`; único fallo aislado en `t_audio` escenario 3 (`device stall`, Mac sin salida AudioUnit activa).
  * `swiftc -typecheck` iOS (`0` errores), `compile_check.py` (`265 files, 0 failed`), `dm/ios/tests` Python (`8` + `48` tests `OK`), `DuoHarness` en simulador `iPhone Duo` (`testLiveJoyConMapsSidewaysAndKeepsItsOwnPlayer` y `testLiveJoyConsJoinOnButtonAndKeepPromptInputOutOfGame`: `2 tests, 0 failures`), `test_dmsh_abi` (`11 OK`), `test_dmsh_alpha` (`8 OK`), y `dm/tools/tests` (`test_env_switches`, `test_lint_c`, `test_launcher_settings`, `test_settings`: `40 OK`). `test_docs.py` (`75 OK`, conservando 1 fallo en `test_relative_links_and_anchors_resolve` por 13 referencias previas de upstream a archivos/secciones no incluidos en el fork).

### 16.6 Compilación Release e Instalación en Dispositivo (`iPhone Dams` — 2026-11-20)
* **Compilación Release en Xcode:**
  * Binario generado en: `/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app`
  * Firma: `Apple Development: Marco Astorga (L4J24857AX)`, entitlements `com.apple.developer.kernel.increased-memory-limit = true`.
  * Verificación de bundle: `DMCommit = 2e09fab66a20`, `DMChain = 39417042cfe8`, `CFBundleIdentifier = com.marcoastorga.zeldaoot`, `DMShowFPS = YES`.
  * Recursos empaquetados: `mods_rstb.bin` presente (`1,761,481` bytes), `romfs.id` presente sin RomFS interna (`DM_COPY_ROMFS=NO`), 27 ajustes en `dm/launcher.json`.
* **Instalación en iPhone 17 Pro Max (`iPhone Dams` `00008150-00190C880E3B401C`):**
  * Instalado encima vía `xcrun devicectl device install app` con código de salida `0` (`databaseSequenceNumber: 2580`).
  * Ruta en sandbox: `/private/var/containers/Bundle/Application/D9A9C145-2459-49ED-A3B8-F656BD0071B4/DmIOS.app/`.
  * Preservación estricta: `Documents/RomFS.img` (`24.44 GB`) y `Documents/save/` intactos sin desinstalación.
  * Espacio libre en Mac tras la operación: `87 GiB` disponibles en `/System/Volumes/Data`.



