#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Select Vulkan ICDs for the packaged port before starting GTK or game."""
import hashlib, json, os, sys
from pathlib import Path

_cur = Path(__file__).resolve().parent
if str(_cur) not in sys.path:
    sys.path.insert(0, str(_cur))
from vulkan_host import host_nvidia, elf64

MANIFEST_DIRS = (
    Path("/etc/vulkan/icd.d"), Path("/usr/share/vulkan/icd.d"),
    Path("/usr/local/share/vulkan/icd.d"), Path("/run/opengl-driver/share/vulkan/icd.d"),
)
LIBRARY_DIRS = (
    Path("/usr/lib/x86_64-linux-gnu"), Path("/lib/x86_64-linux-gnu"),
    Path("/usr/lib64"), Path("/lib64"), Path("/usr/lib"), Path("/lib"),
    Path("/run/opengl-driver/lib"),
)

def configure(env, manifest_dirs=MANIFEST_DIRS, library_dirs=LIBRARY_DIRS):
    if env.get("VK_DRIVER_FILES") or env.get("VK_ICD_FILENAMES"):
        return "Vulkan: using explicit driver override"
    bundled = env.get("BB_BUNDLED_VK_DRIVER_FILES", "")
    if env.get("VK_ADD_DRIVER_FILES"):
        bundled = env["VK_ADD_DRIVER_FILES"] + (":" + bundled if bundled else "")
    extra = env.get("BB_NVIDIA_LIB_DIR")
    lib_dirs = (Path(extra), *library_dirs) if extra else library_dirs
    found = host_nvidia(manifest_dirs, lib_dirs)

    if not found:
        if bundled:
            env["VK_DRIVER_FILES"] = bundled
        return "Vulkan: bundled AMD/Intel drivers; no accessible NVIDIA ICD found"

    data, driver = found
    key = hashlib.sha256(f"{driver}:{driver.stat().st_mtime_ns}".encode()).hexdigest()[:16]
    base_data = env.get("BB_DATA_DIR") or str(Path(env.get("XDG_DATA_HOME", str(Path.home() / ".local/share"))) / "bbport")
    cache = Path(base_data) / "vulkan" / "nvidia" / key
    libraries = cache / "lib"
    libraries.mkdir(parents=True, exist_ok=True)
    sources = [driver]
    for pattern in ("libnvidia-*.so*", "libGLX_nvidia.so*", "libEGL_nvidia.so*"):
        sources.extend(sorted(driver.parent.glob(pattern)))
    for s in sources:
        if elf64(s):
            try:
                (libraries / s.name).symlink_to(s.resolve())
            except FileExistsError:
                pass
    data["ICD"]["library_path"] = str(libraries / driver.name)
    manifest = cache / "nvidia_icd.json"
    manifest.write_text(json.dumps(data, indent=2) + "\n")
    env["VK_DRIVER_FILES"] = str(manifest) + (":" + bundled if bundled else "")
    env["LD_LIBRARY_PATH"] = str(libraries) + (":" + env["LD_LIBRARY_PATH"] if env.get("LD_LIBRARY_PATH") else "")
    return f"Vulkan: host NVIDIA driver {driver}; bundled AMD/Intel also available"

def main():
    if len(sys.argv) < 2:
        return 1
    try:
        print(configure(os.environ), file=sys.stderr, flush=True)
    except OSError as e:
        print(f"bbport: cannot prepare NVIDIA driver: {e}", file=sys.stderr)
        return 1
    os.execvpe(sys.argv[1], sys.argv[1:], os.environ)

if __name__ == "__main__":
    sys.exit(main())
