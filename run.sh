#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"

source scripts/run_env.sh "$@"
game=$BB_RESOLVED_GAME
data=${BB_DATA_DIR:-.}
out=$data/out
mkdir -p "$out"
export BB_CONFIG=${BB_CONFIG:-$data/bbport.ini}
if [[ -z ${BB_FSR411_DIR:-} && ! -d fsr4_411 && -d $data/fsr4_411 ]]; then
    export BB_FSR411_DIR=$data/fsr4_411
fi

original_game=$game
game=$("$PYTHON" scripts/mods.py "$game" --out "$out" \
    --mods-dir "${BB_MODS_DIR:-$data/mods}" --config "${BB_MODS_CONFIG:-$data/mods.json}" \
    --enabled "${BB_MODS_ENABLED:-1}")
if [[ $game != "$(realpath "$original_game")" ]]; then
    mod_game=$game
    trap '"$PYTHON" -c '\''import shutil,sys; shutil.rmtree(sys.argv[1])'\'' "$mod_game"' EXIT
fi

"$PYTHON" scripts/prepare.py "$game" --out "$out"
"$PYTHON" scripts/link_libc.py "$game" --out "$out"
"$PYTHON" scripts/link_modules.py "$game" --out "$out"
"$PYTHON" scripts/content_profile.py "$game" --out "$out" --sku "${BB_CONTENT_SKU:-full}"

if [[ ${BB_AUTO_RENDER_RES:-} == 1 ]]; then
    unset BB_RENDER_RES BB_OUTPUT_RES BB_AUTO_RENDER_RES
fi
fps=${BB_FPS:-uncap}

source scripts/run_resolution.sh

"$PYTHON" scripts/patches.py --out "$out" --fps "$fps" --extra "${BB_PATCHES:-}" \
    --settings "$BB_CONFIG" --game-dir "$game" --render-res "${BB_RENDER_RES:-}" \
    --output-res "${BB_OUTPUT_RES:-}" --patches-dir "${BB_PATCHES_DIR:-$data/patches}" \
    --patches-config "${BB_PATCHES_CONFIG:-$data/patches.json}"

if [[ -z ${BB_VBLANK_HZ:-} ]]; then
    case $fps in uncap) export BB_VBLANK_HZ=0 ;; 90) export BB_VBLANK_HZ=90 ;; *) export BB_VBLANK_HZ=60 ;; esac
fi

if [[ -z ${BB_PREBUILT:-} && -d fsr4_shaders ]] && command -v spirv-cross >/dev/null; then
    bash tools/fsr4_optimize.sh || echo 'FSR 4: optimized post passes not built' >&2
fi

if [[ -n ${BB_PREBUILT:-} ]]; then
    probe=${BB_PROBE:-bin/bb-probe}
else
    bash build.sh
    probe=out/bb-probe
fi

probe_args=("$out/boot-linked.bin" --content-profile "$out/content.bin" --patches "$out/patches.bin" \
    --app0 "$game" --user "${BB_USER_DIR:-$data/user}" --timeout "${BB_TIMEOUT:-0}" "$@")

if [[ -n ${mod_game:-} ]]; then
    "$probe" "${probe_args[@]}" &
    mod_pid=$!
    trap 'kill -TERM "$mod_pid" 2>/dev/null || true' TERM INT
    mod_status=0
    wait "$mod_pid" || mod_status=$?
    if kill -0 "$mod_pid" 2>/dev/null; then wait "$mod_pid" || mod_status=$?; fi
    exit "$mod_status"
fi

exec "$probe" "${probe_args[@]}"
