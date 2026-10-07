#!/usr/bin/env bash
# Resolution resolution and live resolution detection for run.sh
set -euo pipefail

if [[ -z ${BB_RENDER_RES:-} ]]; then
    read -r scaled_render scaled_output < <("$PYTHON" scripts/patches.py --print-scaled --settings "$BB_CONFIG") || true
    scaled_output=${scaled_output%$'\r'}
    scaled_render=${scaled_render%$'\r'}
fi
live=0
if [[ -n ${scaled_output:-} ]]; then
    live=${BB_LIVE_RES:-}
    if [[ -z $live && -f $BB_CONFIG ]]; then
        while IFS= read -r line || [[ -n $line ]]; do
            line=${line%$'\r'}
            [[ $line =~ ^live_resolution=([01]|auto)$ ]] && live=${BASH_REMATCH[1]}
        done < "$BB_CONFIG"
    fi
    if [[ $live == auto ]]; then
        if [[ -n ${BB_PROBE:-} ]]; then caps=$(dirname -- "$BB_PROBE")/bb-gpu-capabilities
        elif [[ -n ${BB_PREBUILT:-} ]]; then caps=bin/bb-gpu-capabilities
        else caps=out/bb-gpu-capabilities; fi
        live=$("$caps" --live-resolution 2> >(while IFS= read -r line; do
            [[ $line == *MANGOHUD* ]] || printf '%s\n' "$line"; done >&2)) || live=0
        live=${live%$'\r'}
    fi
    [[ $live == 1 ]] || live=0
fi

if [[ $live == 1 ]]; then
    echo "Output ${scaled_output}: live resolution changes (live_resolution=0: startup patch)"
elif [[ -n ${scaled_output:-} ]]; then
    export BB_RENDER_RES=$scaled_render BB_OUTPUT_RES=$scaled_output BB_AUTO_RENDER_RES=1
    export BB_DMEM_MB=${BB_DMEM_MB:-9152}
    echo "Output ${scaled_output}: scene ${scaled_render}, direct memory ${BB_DMEM_MB} MiB (live_resolution=1: live changes)"
fi
