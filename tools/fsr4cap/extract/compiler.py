# SPDX-License-Identifier: GPL-2.0-or-later
"""SPIR-V translation and output serialization for extracted sets."""

import hashlib
import os
import subprocess
import sys
from .rules import FLAGS


def compile_sets(sets, out, dxil_spirv):
    postpass_script = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'postpass_lds.py')
    for key, entry in sorted(sets.items()):
        d = os.path.join(out, key)
        os.makedirs(d, exist_ok=True)
        for name, path in entry['dxil'].items():
            spv = os.path.join(d, f'{name}.spv')
            subprocess.run([dxil_spirv, path, *FLAGS, '--output', spv], check=True, stderr=subprocess.DEVNULL)
            if name == 'postpass':
                os.replace(spv, os.path.join(d, 'postpass_orig.spv'))
                asm = subprocess.run(['spirv-dis', os.path.join(d, 'postpass_orig.spv')], check=True,
                                     capture_output=True, text=True).stdout
                lds = subprocess.run([sys.executable, postpass_script],
                                     input=asm, check=True, capture_output=True, text=True).stdout
                subprocess.run(['spirv-as', '--target-env', 'spv1.3', '-', '-o', spv], input=lds, check=True,
                               text=True)
        open(os.path.join(d, 'initializer.bin'), 'wb').write(entry['init'])
        h = hashlib.sha256(entry["init"]).hexdigest()[:12]
        print(f'{key}: {len(entry["dxil"])} shaders, initializer {h}, from {len(entry["caps"])} captures')
