# SPDX-License-Identifier: GPL-2.0-or-later
"""Helper for executing restart resolution test harness."""

import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
from paths import BASH, ROOT


def run_restarts(explicit=False, live=False, ini_extra='', caps=None, bare_path=False):
    with tempfile.TemporaryDirectory() as directory:
        data = Path(directory)
        out = data / 'out'
        out.mkdir()
        elf = bytearray(120)
        struct.pack_into('<Q', elf, 0x20, 64)
        struct.pack_into('<HH', elf, 0x36, 56, 1)
        struct.pack_into('<IIQQQQQQ', elf, 64, 1, 0, 0, 0, 0, 0, 0x6000000, 0)
        (out / 'eboot.elf').write_bytes(elf)
        (data / 'eboot.bin').touch()
        config = data / 'bbport.ini'
        config.write_text('upscaler=fsr3\npreset=1\noutput_res=1280x720\n' + ini_extra)
        if caps is not None:
            tool = data / 'bb-gpu-capabilities'
            tool.write_text(f'#!/bin/sh\necho {caps}\n')
            tool.chmod(0o755)
        python = data / 'python'
        python.write_text(f'#!{sys.executable}\n' +
            'import subprocess, sys\n'
            'if sys.argv[1] in ("scripts/patches.py", "scripts/mods.py"):\n'
            '    sys.exit(subprocess.call([sys.executable, *sys.argv[1:]]))\n')
        python.chmod(0o755)
        probe = data / 'probe'
        probe.write_text(f'#!{sys.executable}\n' +
            'import json, os, subprocess, sys\n'
            'from pathlib import Path\n'
            'config=Path(os.environ["BB_CONFIG"])\n'
            'stage=int(os.environ.get("BB_TEST_STAGE", "0"))\n'
            'with (config.parent/"environments").open("a") as f:\n'
            '    f.write(json.dumps({key:os.environ.get(key) for key in '
            '("BB_RENDER_RES", "BB_OUTPUT_RES", "BB_AUTO_RENDER_RES")})+"\\n")\n'
            'if stage<2:\n'
            '    config.write_text("upscaler=fsr3\\npreset=4\\noutput_res="+'
            '("1280x720" if stage==0 else "1920x1080")+"\\n")\n'
            '    os.environ["BB_TEST_STAGE"]=str(stage+1)\n'
            f'    sys.exit(subprocess.call([r"{BASH}", "run.sh"]))\n')
        probe.chmod(0o755)
        env = dict(os.environ, BB_PREBUILT='1', BB_PROBE=str(probe), PYTHON=str(python),
                   BB_DATA_DIR=str(data), BB_CONFIG=str(config), BB_GAME_DIR=str(data))
        for key in ('BB_RENDER_RES', 'BB_OUTPUT_RES', 'BB_AUTO_RENDER_RES', 'BB_TEST_STAGE'):
            env.pop(key, None)
        env.pop('BB_LIVE_RES', None)
        if explicit: env['BB_RENDER_RES'] = '800x450'
        if live: env['BB_LIVE_RES'] = '1'
        if bare_path:
            tools = data / 'bin'
            tools.mkdir()
            for name in ('bash', 'dirname', 'mkdir', 'realpath'):
                which = shutil.which(name)
                if which: (tools / name).symlink_to(which)
            env['PATH'] = str(tools)
        subprocess.run([BASH, 'run.sh'], cwd=ROOT, env=env,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True, timeout=30)
        return [json.loads(line) for line in (data / 'environments').read_text().splitlines()]
