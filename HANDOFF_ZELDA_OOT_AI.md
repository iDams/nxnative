# HANDOFF TÉCNICO: The Legend of Zelda: Ocarina of Time (Switch 2 en iOS)

> **Reglas obligatorias del proyecto:** [`REGLAS.md`](REGLAS.md) — icono (`/Users/marco/Ocarina/assets/icon.jpeg`), limpieza y espacio (>20 GB libres, nunca duplicar el RomFS), instalación encima de la app (`DM_COPY_ROMFS=NO`) y cadena de shaders fija (`39417042cfe8`).

> **Documentación de sesiones previas:** [`DOCUMENTACION_SESION_2026-11-08.md`](DOCUMENTACION_SESION_2026-11-08.md).

---

> [!IMPORTANT]
> **VERSIÓN ESTABLE DE REFERENCIA (`v1.0-oot-30fps-stable` — 2026-11-09):**
> * **Tag Git en `/Users/marco/ns` (`origin`):** `v1.0-oot-30fps-stable` (también respaldado en la rama `stable/oot-30fps`).
> * **Cadena de Shaders:** `39417042cfe8` (`/Users/marco/Ocarina/shader/ios/b-39417042cfe8.img`, 5.4 GB, 42,848 shaders, `inputs 4bd48f723876`).
> * **Rendimiento Medido en iPhone 17 Pro Max (Bosque Kokiri):** **30.0 FPS estables clavados** (`frame p50 = 33.33 ms`, `p95 = 33.91 ms`, 0–1 frames caídos por cada 300 frames, consumo CPU ~2.1 W, estado térmico `0`).
> * **Estado Gráfico:** Hierba, follaje, sombras y geometría completamente estables sin parpadeos ni objetos que aparezcan/desaparezcan al mover la cámara.
> * **Cómo volver a esta versión estable si un experimento futuro falla:** Ver [Sección 8 (Restauración Rápida a la Versión Estable)](#8-restauración-rápida-a-la-versión-estable-rollback).

---

## 1. Repositorios Base, Referencias Comunitarias y Puntos de Restauración

| Repositorio / Fuente | URL Remota | Rama / Tag Estable | Rol |
| :--- | :--- | :--- | :--- |
| **nxnative (Fork Privado)** | `https://github.com/iDams/nxnative.git` | `main` / Tag: **`v1.0-oot-30fps-stable`** / Rama: **`stable/oot-30fps`** | Motor nativo de recompilación para Switch / Switch 2 en Apple Silicon / iOS. Clonado local en `/Users/marco/ns`. |
| **nxnative (Upstream)** | `https://github.com/seedds/nxnative.git` | `main` (incluye [PR #1852](https://github.com/seedds/nxnative/pull/1852)) | Repositorio upstream original del motor `dm` (revisar PRs de rendimiento, Metal 4 y `0400486024DEC000`). |
| **kid-with-sword-remake (cheemu21)** | `https://github.com/cheemu21/kid-with-sword-remake/releases` | Releases (`v22+`) | Builds y parches comunitarios específicos para *Zelda: Ocarina of Time* (Switch 2). Revisar periódicamente sus notas de release y commits embebidos para nuevas optimizaciones de OoT. |
| **NX2Apple (Frontend & Perfiles)** | `https://github.com/LNPANDY/NX2Apple` | `main` | Frontend comunitario y parches iOS sobre `nxnative`. |
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
* **`Documents/env.txt`**: Configuración activa de rendimiento a 30 FPS.

---

## 5. Configuración Estable de `Documents/env.txt` en el iPhone

```ini
DM_OPMODE=0
DM_SHOW_FPS=1
DM_LAUNCHER=0
DM_FPS=30
DM_FPS_TARGET=30
DM_FPS_HOLD=off
DM_FPS_PAYBACK=off
DM_DYNAMIC_RESOLUTION=1
DM_PSO_ASYNC=1
DM_THERMAL_CAP=0
DM_UPSCALER=metalfx
DM_QOS_MAP=1
DM_QOS_EDGES=45,59
```

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
# 1. Restaurar el código del motor al tag estable v1.0-oot-30fps-stable
cd /Users/marco/ns
git checkout v1.0-oot-30fps-stable
# (O para resetear main a la versión estable: git checkout main && git reset --hard v1.0-oot-30fps-stable)

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

