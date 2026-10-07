#!/usr/bin/env bash
# Environment setup and game path detection helper for run.sh
set -euo pipefail

if [[ ${1:-} == --software ]]; then
    shift
    if [[ -z ${VK_DRIVER_FILES:-} ]]; then
        for candidate in /run/opengl-driver/share/vulkan/icd.d/lvp_icd*.json /usr/share/vulkan/icd.d/lvp_icd*.json; do
            if [[ -f $candidate ]]; then export VK_DRIVER_FILES=$candidate; break; fi
        done
    fi
    if [[ -z ${VK_DRIVER_FILES:-} ]]; then echo 'Lavapipe not found; set VK_DRIVER_FILES.' >&2; exit 1; fi
    export VK_LOADER_LAYERS_DISABLE='~implicit~'
fi

if [[ -z ${BB_PREBUILT:-} && -z ${BB_IN_NIX_SHELL:-} ]] && ! { command -v pkg-config >/dev/null && pkg-config --exists vulkan sdl3; } && command -v nix-shell >/dev/null; then
    args=''; if (( $# )); then args=$(printf '%q ' "$@"); fi
    exec env BB_IN_NIX_SHELL=1 nix-shell shell.nix --run "bash run.sh $args"
fi

if [[ -z ${PYTHON:-} ]]; then
    PYTHON=$(command -v python3 || true)
    if [[ -z $PYTHON ]]; then
        for candidate in /nix/store/*-python3-*/bin/python3; do
            if [[ -x $candidate ]]; then PYTHON=$candidate; break; fi
        done
    fi
fi
if [[ -z ${PYTHON:-} ]]; then echo 'Install Python 3 or set PYTHON.' >&2; exit 1; fi
export PYTHON

game=${BB_GAME_DIR:-}
if [[ -z $game ]]; then
    for cusa in CUSA00900 CUSA03173 CUSA00207 CUSA00208 CUSA03023 CUSA01363; do
        if [[ -f "../$cusa/eboot.bin" ]]; then game="../$cusa"; break; fi
        if [[ -f "$cusa/eboot.bin" ]]; then game="$cusa"; break; fi
    done
fi
if [[ -z $game || ! -f $game/eboot.bin ]]; then
    echo "No eboot.bin found. Please set BB_GAME_DIR to your game directory (e.g. CUSA00900 or CUSA03173)." >&2
    exit 1
fi
export BB_RESOLVED_GAME=$game
