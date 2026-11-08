# Documentación de sesión — Zelda: Ocarina of Time (Switch 2) en iPhone/Mac

Fecha: **2026-11-08**. Documento de continuidad: todo lo que se hizo, el estado exacto y lo que queda abierto.

## 1. Objetivo

Ejecutar *The Legend of Zelda: Ocarina of Time* (Switch 2, `0400486024DEC000`, update `v131072`, SDK 22.2.8) de forma nativa en iPhone 17 Pro Max y Mac con el motor **nxnative** (`/Users/marco/ns`), usando el perfil y los shaders del juego.

## 2. Repositorios y commits

| Repo | Uso | Estado |
|---|---|---|
| `seedds/nxnative` (upstream) | motor `dm` | **`7f502220`** aplicado sobre `37f076a5` en el worktree `/Users/marco/Ocarina/ns_7f` (+ cambios locales y el parche NX2Apple 0096). |
| `iDams/nxnative` | fork principal local | `origin` de `/Users/marco/ns`. |
| `LNPANDY/NX2Apple` | frontend/parches iOS | pin `88c7e03f`; revisado `main` `734bc63d` (v1.3.0). Se tomaron táctil/quit/layout, icono/estilo y el parche **0096** (mesh grids de 1024, page faults de OoT). |

Cadena de shaders actual: **`39417042cfe8`** (no cambia con los 7 commits nuevos). La biblioteca se guarda en un solo `b-<chain>.img`.

## 3. Historial del motor en esta sesión

1. `88c7e03f` (pin NX2Apple 1.2.3) → primer build iOS/Mac estable; se implementó a mano el método que faltaba del codificador diferido (`drawMeshThreadgroupsWithIndirectBuffer...`), hoy oficial en upstream.
2. `5636e7a7` (31 commits): LDSM con barreras de subgrupo (blanco del bosque), HMMA, texturas desde su storage (agujeros de terreno), tabla de pools incremental, storage takeover, raw memory, codificador diferido oficial. Cadena `d866f9700d78`.
3. `581f9c1d` (9 commits): fix de follaje M3 (CAS loop de 8,3–8,6 ms → un store), depth pass de OoT, `fps_target 60`, frame step del host, resolución dinámica a full. Cadena `daf7512cdfc6`.
4. `37f076a5` (71 commits): productos extendidos escalares (Kokiri Forest 59,1 fps en M4 Pro), **resolución completa**, cargas raw de 128 bits, white blotches de follaje/mesh, niebla, page faults, pipeline archive, frame step. Cadena `39417042cfe8`.
5. **`7f502220` (7 commits) aplicado** en `/Users/marco/Ocarina/ns_7f`, conservando iOS táctil/quit, standby, NDVE, icono y `op_mode handheld`: render targets de la resolución dinámica en heaps propios (3 texturas nuevas/frame; +455–521 MB de residency por 300 frames; p90 33–50 ms → 17–18 ms); `7b6506dd` crash de Kokiri Forest en AGX prepareForEnqueue; `7de11da5` controlador de resolución dinámica por carga medida (`DM_DYNAMIC_RESOLUTION=1`). La cadena sigue `39417042cfe8` e `inputs` sigue `4bd48f723876`.
6. **Parche NX2Apple 0096 aplicado** (no está en `7f502220`, upstream PR #1708): divide los mesh draws indirectos sin task shader en partes de 1024 workgroups. OoT emite hasta 1.771 workgroups en hierba/foliage y sin esto la GPU hace page faults a los 30–40 s de cargar la partida.

## 4. Perfil `dm/games/0400486024DEC000/`

- `mem_reserve 0x320000000`, `mem_alias 0x100000000`, `mem_stack 0x10000000`, `app_pool 0x240000000` (9 GB), `mem_map_limit on`, `sys_resource 0x3000000`.
- `render_cond on`, `gpu_timestamps segment`, `draw_barriers game`, `tex_flush_upload on`, `tex_dirty_upload on`.
- `op_mode handheld` (añadido local), `fps_target 60` (upstream 581+), `pad_layout full`.
- `patches.py`: corte NDVE (recurso NDVE nulo → `MovieRenderer`; título 20→60 fps) + **frame step** (ratio del host) + **dynamic resolution** (0 = full; 1 = escalada por carga medida). El parche nuevo de `7de11da5` sustituye el controlador de resolución dinámica.
- `shader_map.tsv` 42.848 shaders (mapa upstream 37f), `pso_manifest.tsv`, `shader_pairs.tsv`, `pso_pairs.tsv`, `bench.tsv`.
- `c1_full.tsv`/`cb_extent.tsv`: generados con `c1_guard.py`/`cb_extent.py`; deben nombrar la cadena y `inputs` de la biblioteca. **Sin c1, el host usa 64 KiB por c1 y pierde rendimiento.**

## 5. Cambios locales importantes (no upstream)

- **iOS táctil/quit**: `DmGamepad.m` (geometría `DmPadGeometry.h`, tamaños, stick, layout), `quit_prompt.h`, `front_ios.m`, `GameMenu.swift`, `DmGameMenu.h`, tests. Fusionados con las novedades 37f de `DmGamepad` (`ctrl_prompt`, press-to-join) para no perder `front_ctrl_support`.
- **Mac standby**: en `dm/host/front_mac.m` se añadió suspensión real en sueño de sistema/pantalla, bloqueo, sesión, app oculta, ventana tapada y minimizada (`g_susp`, `dm_audio_suspend + dm_gfx_suspend`). Antes solo perdía foco y seguía renderizando; los render targets móviles sumaban ~2 GB de texturas por 300 frames. El Mac se eliminó después para liberar espacio.
- **Icono**: `/Users/marco/Ocarina/assets/icon.jpeg` (copia de `/Users/marco/Downloads/images.jpeg`, 554×554). Se pasa como `DM_ICON`; no usar `extract/icon.jpg`.
- **Reglas**: `REGLAS.md` en `Ocarina` y `ns` (icono, limpieza/espacio, RomFS/saves, instalación encima).

## 6. Artefactos y rutas

- Motor: `/Users/marco/Ocarina/ns_7f` (worktree en **`7f502220`** + cambios locales + parche 0096); `/Users/marco/ns` sigue en `37f076a5` + locales hasta consolidar.
- Perfil: `/Users/marco/ns_7f/dm/games/0400486024DEC000/` (mismas tablas e `inputs 4bd48f723876`).
- Módulos reescritos: `/Users/marco/Ocarina/patched_7f/` (`main`, `subsdk0`, `sdk`; mismo `PATCH_OFF`).
- Paquete de shaders de entrada: `/Users/marco/Ocarina/shader/pkg_5636` (516 MB) + `tables_5636`.
- Biblioteca iOS **completa** (42.848 shaders, **ok 42.848**): `shader/ios/b-39417042cfe8.img` (5,4 GB, 130.466 archivos, sello `game 0400486024DEC000` / `inputs 4bd48f723876` **dentro** de la imagen). `c1_full.tsv` (650 no acotados) y `cb_extent.tsv` (39.125 shaders) generados por el build.
- App instalada: `/Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app` (**5,5 GB**, icono nuevo, c1/cb, host .922, `7f502220+dirty`).
- RomFS: `/Users/marco/Ocarina/extract/v131072/RomFS.img` (symlink al volumen externo) y `Documents/RomFS.img` en el iPhone (24,4 GB). **No copiar de nuevo.**
- Partidas: `save/`, `save_mac/` y `Documents/save` del iPhone.
- Handoff: `HANDOFF_ZELDA_OOT_AI.md` en `ns` y `Ocarina`.

## 7. Comandos de build

```bash
# Host Mac (perfil + cadena actual)
cd /Users/marco/ns
make -C dm/host GAME=0400486024DEC000 -j8 dmhost

# Reescribir módulos con los parches del perfil
OUT=/Users/marco/Ocarina/patched_37f
for m in main subsdk0 sdk; do
  DM_GAME=0400486024DEC000 .venv/bin/python3 dm/tools/patch_nso.py \
    /Users/marco/Ocarina/extract/v131072/ExeFS/$m "$OUT" $m
done

# Biblioteca iOS (parcial, set de la ruta)
KEYS=$(awk '!/^#/ && NF {print $1}' dm/games/0400486024DEC000/shader_seen.tsv)
DOTNET_ROOT="$HOME/.dotnet" DM_GAME=0400486024DEC000 DM_ALLOW_OTHER_METAL=1 \
DMSH_TARGETS=ios IOSOUT=/Users/marco/Ocarina/shader/ios J=8 \
  .venv/bin/python3 dm/gfx/tools/own_compilers.py \
  /Users/marco/Ocarina/shader/pkg_5636 /Users/marco/Ocarina/shader/cache_ios $KEYS

# Biblioteca iOS completa (42.848; sin KEYS)
DOTNET_ROOT="$HOME/.dotnet" DM_GAME=0400486024DEC000 DM_ALLOW_OTHER_METAL=1 \
DMSH_TARGETS=ios IOSOUT=/Users/marco/Ocarina/shader/ios J=8 \
  .venv/bin/python3 dm/gfx/tools/own_compilers.py \
  /Users/marco/Ocarina/shader/pkg_5636 /Users/marco/Ocarina/shader/cache_full

# c1/cb de una imagen (la imagen iOS solo trae metallibs; usar la Mac de cache)
DM_GAME=0400486024DEC000 .venv/bin/python3 dm/gfx/tools/c1_guard.py <imagen> /tmp/c1.tsv
DM_GAME=0400486024DEC000 .venv/bin/python3 dm/gfx/tools/cb_extent.py <imagen> /tmp/cb.tsv
# Editar la cabecera a: "# chain b-<chain> inputs <inputs> ..."
# Sellar la imagen metiendo inputs.tsv dentro (shader_lib.write), NO como carpeta:
# una carpeta b-<chain>/ con inputs.tsv hace que shader_lib use la carpeta y busque metallibs sueltos.

# Proyecto Xcode + app
GAME=0400486024DEC000 W=/Users/marco/Ocarina \
PATCHED=/Users/marco/Ocarina/patched_37f \
ROMFS=/Users/marco/Ocarina/extract/v131072/RomFS.img \
MSL_IOS=/Users/marco/Ocarina/shader/ios DM_COPY_ROMFS=NO \
DM_BUNDLE_ID=com.marcoastorga.zeldaoot DM_TEAM_ID=TGU3UHC7C7 \
DM_DISPLAY_NAME="Zelda Ocarina of Time" DM_APP_VERSION=1.1.1 \
DM_APP_CATEGORY=public.app-category.adventure-games \
DM_SHOW_FPS=1 DM_LANG=en-US \
DM_ICON=/Users/marco/Ocarina/assets/icon.jpeg \
FONT_PY=/Users/marco/ns/.venv/bin/python3 DOTNET_ROOT="$HOME/.dotnet" \
  dm/ios/tools/build_ipa.sh --prepare-xcode

xcodebuild -project /Users/marco/Ocarina/xcode/DmIOS.xcodeproj \
  -scheme "Zelda Ocarina of Time (0400486024DEC000)" \
  -configuration Release -destination 'generic/platform=iOS' \
  -allowProvisioningUpdates build
```

Instalación (encima, sin desinstalar; conserva RomFS y saves):

```bash
xcrun devicectl device install app --device 00008150-00190C880E3B401C \
  /Users/marco/Ocarina/build/ios/xcode-dd/Build/Products/Release-iphoneos/DmIOS.app
```

## 8. Estado de la instalación y pruebas

- App instalada en iPhone Dams (`com.marcoastorga.zeldaoot`), build **`7f50222003e4+dirty`**, biblioteca **completa** (5,4 GB), c1/cb activos (`uniform read extents: 39125 shaders`), host shaders .922 y heaps reciclados (`recycled render-target heaps made 1157, reused 1859, freed 547, waiting 12 (20 MB)`).
- **Cero `failed to load shader`** y cero crashes: el problema de los shaders ausentes está resuelto. Kokiri Forest se ve correcto (sin blanco LDSM, sombras/texturas/terreno bien).
- **Jugando: 20–30 fps** (antes ~7). Medido en frames 1200–4500: frame p50 **33,3–35,8 ms**, p95 50,2–50,9 ms; GPU p50 **14,3–16,6 ms**, p95 15,9–21,9 ms; NVN submit+present p50 **12,3–18,5 ms**; footprint 3,8–4,0 GB; headroom 4,2 GB; thermal 0.
- El frame step oscila entre 1, 2 y 3 vsyncs (60/30/20 fps) porque el coste CPU (~18,5 ms de encode) no cabe en 16,7 ms; el probe a hold 1 falla y el juego pasa tiempo a 3 vsyncs: game time **0,893** de tiempo real en la ventana medida (384/136/1280 frames a 1/2/3 vsyncs, 586 late, 841 dropped). No es un problema de shaders.
- `Documents/env.txt` lleva **`DM_DYNAMIC_RESOLUTION=1`** desde el 2026-11-08 16:36 para probar el controlador nuevo (7de11da5) en el siguiente arranque. La partida actual sigue a resolución completa (`DM_DYNAMIC_RESOLUTION=0` al arrancar).
- El `codesign` del `.app` puede quedar inválido después del script de recursos; re-firmar con el `.xcent` de Xcode y verificar antes de instalar. La app instalada verifica `SIGN_OK`.
- Metal Toolchain 27B47 (.922) instalado con `xcodebuild -downloadComponent MetalToolchain`; con él los **42.848 shaders compilan `ok 42.848` / 0 fallos** y el host compila sus 13 librerías.

## 9. Memoria / standby

- iPhone: `DM_GUEST_FILE=1` forzado en `main.m`; 13 archivos sparse de 1 GB, `phys_footprint` 3,8–4,0 GB, headroom al límite del sistema **4,2 GB**, sin Jetsam. El reciclado de render targets de `7f502220` se mantiene acotado (1.157 heaps hechos, 1.859 reutilizados, 547 liberados, 12 esperando / 20 MB).
- Mac (cuando se usaba): `footprint` 9,2–9,3 GB; el consumo de 20 GB anterior era el bug de memoria, no el juego.
- Standby Mac corregido en `front_mac.m` (suspensión real); el Mac se eliminó después para liberar espacio.
- `APP_POOL` debe seguir en `0x240000000` (9 GB); reducirlo rompe el `StandardAllocator`.

## 10. Espacio y limpieza

- Se borraron caches/cadenas viejas, `shader/mac`, `build/ios` viejo, `patched_5636`, `dmhost`, `run_mac.command`, `temp_mac`, el IPA viejo y `/Users/marco/Downloads/917504` (22 GB, Super Mario 3D World + Bowser's Fury extraído, no usado).
- **~120 GB libres** al escribir esto (la app instalada son 5,5 GB y la biblioteca 5,4 GB). Reglas en `REGLAS.md`.
- No borrar: RomFS, `save`/`save_mac`, perfil actual, `pkg_5636`, `tables_5636`, `patched_7f`, biblioteca iOS completa, handoff.
- Limpieza pendiente cuando se cierre la puesta a punto: `shader/cache_ios` (restos del set parcial, ~5,4 GB), `shader/cache_full` (~8,7 GB de CAS; ya generó el `.img` y c1/cb), `patched_37f` y el worktree `ns_7f` tras consolidar en `ns`.

## 11. Consolidación en `ns` y corrección de 0 FPS / 20 FPS (commits `c178a59c` y `3d0351d7`)

1. **Consolidado `7f502220` + locales + parche 0096 en `/Users/marco/ns`** (`c178a59c`), eliminado el worktree temporal `/Users/marco/Ocarina/ns_7f`, y subido a `origin/main` (`https://github.com/iDams/nxnative.git`).
2. **Eliminación de caídas a 0 FPS (`3d0351d7`):**
   - Fusionados los 470 nuevos registros de `Documents/pso_user.tsv` del iPhone (`5` shaders, `403` render PSOs, `62` compute PSOs) en `dm/games/0400486024DEC000/pso_manifest.tsv` (total: 10.098 entradas).
   - Activada compilación asíncrona no bloqueante de render PSOs en `pso_get_` (`pso_async_on`, por defecto en iOS; `DM_PSO_ASYNC=0` la desactiva) + prebuild de compute PSOs en `mtl_prog_prefetch`, evitando bloqueos de 0,4–3,6 s en el submit thread.
3. **Eliminación del estancamiento en 20 FPS y piso de 30 FPS (`3d0351d7`):**
   - Limitado `kmax = 2` en iOS (`fstep_kmax`, piso de 30 FPS; `DM_FPS_FLOOR=20` permite volver a `hold 3`), impidiendo que el gobernador de energía de iOS vea intervalos de 50 ms (`hold 3`), aparque los hilos en núcleos E y baje el reloj de la GPU del A19 Pro de 15 ms a 44 ms.
   - Ajustado `gslow` en `fstep_tick` para exigir `g_fg.ref >= 0.5` y `(cslow || cpu <= kmin + FSTEP_LATE)`, evitando probes fallidos cada 30 frames contra la referencia de 4 ms del menú 2D.
4. **App reconstruida, firmada e instalada** en iPhone Dams (`00008150-00190C880E3B401C`) con `PATCHED=/Users/marco/Ocarina/patched_7f`, `DM_ICON=/Users/marco/Ocarina/assets/icon.jpeg`, `DM_COPY_ROMFS=NO`.

## 12. Advertencias

- El sello `inputs.tsv` de la biblioteca debe ir **dentro del `.img`** (o en una carpeta que sea el build completo). Una carpeta `b-<chain>/` con solo `inputs.tsv` hace que `shader_lib.view()` use la carpeta y el host busque `<key>_b.metallib` sueltos, fallando todos.
- `DM_COPY_ROMFS=NO`; la app usa `Documents/RomFS.img`. No desinstalar la app.
- `DM_PSO_MTL4=1` y `DM_NVN_ENC_THREAD=0` empeoran; no usarlos.
- El `Metal Toolchain` de Xcode debe ser el .922; si falta, `host_msl.py` deja los shaders del host como fuentes y fallan 85 shaders del juego.
- La biblioteca completa ya está instalada (42.848 shaders, 5,4 GB). El frame step puede bajar a 20/30 fps por coste CPU aunque la GPU tenga margen; no confundir con shaders ausentes (el log ya no muestra `failed to load shader`).
