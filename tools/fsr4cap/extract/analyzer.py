# SPDX-License-Identifier: GPL-2.0-or-later
"""Capture trace analyzer and validation logic."""

import glob
import os
import re
import struct
from .rules import SEQUENCE, expected_groups, shader_name, tensor_sizes


def validate_prepass(cap, data, ow, oh, rw, rh, fail):
    f = struct.unpack('<16f', data[:64])
    u = struct.unpack('<10I', data[64:104])
    want_f = [1 / ow, 1 / oh, ow / rw, oh / rh, rw / ow, rh / oh, None, None,
              1 / rw, 1 / rh, ow, oh, ow, oh, 0, 0]
    for i, w in enumerate(want_f):
        if w is not None and abs(f[i] - w) > 1e-6 * max(1, abs(w)):
            fail(f'{cap} prepass constant {i}: {f[i]} != {w}')
    if (u[0], u[1], u[3], u[4]) != (ow, oh, rw, rh):
        fail(f'{cap} prepass sizes {u[:5]}')


def analyze_captures(root, fail):
    sets = {}
    for cap in sorted(glob.glob(os.path.join(root, 'capture_*'))):
        m = re.search(r'capture_(\d+)x(\d+)_(\d+)x(\d+)$', cap)
        rw, rh, ow, oh = map(int, m.groups())
        tier = 't1080' if ow <= 1920 and oh <= 1080 else 't2160'
        model = 'm1' if ow / rw > 2.5 else 'm0'
        trace = open(os.path.join(cap, 'trace.txt')).read()
        mark = 'MARK frame 1\n' if 'MARK frame 1\n' in trace else 'MARK frame 0\n'
        frame = re.split(r'MARK (?:frame \d+|end)\n', trace.split(mark)[1])[0]
        disp = re.findall(r'DISPATCH #\d+ cs=(\w+) root=\w+ groups=(\d+),(\d+),(\d+)\n((?:  .*\n)*)', frame)
        names = [shader_name(os.path.join(cap, f'cs_{h}.dxil')) for h, *_ in disp]
        if names != SEQUENCE:
            fail(f'{cap}: sequence {names}'); continue
        key = f'{tier}_{model}'
        entry = sets.setdefault(key, {'shaders': {}, 'init': None, 'caps': []})
        entry['caps'].append(os.path.basename(cap))
        for (h, gx, gy, gz, body), name in zip(disp, names):
            groups, want = (int(gx), int(gy), int(gz)), expected_groups(name, rw, rh, ow, oh)
            if want and want != groups: fail(f'{cap} {name}: groups {groups}, rule {want}')
            prev = entry['shaders'].get(name)
            if prev and prev != h: fail(f'{cap} {name}: shader {h} differs from {prev} in the same set')
            entry['shaders'][name] = h
            entry.setdefault('dxil', {})[name] = os.path.join(cap, f'cs_{h}.dxil')
            cbv = re.search(r'CBV r\d+\+\d+ data=(data_\w+\.bin)', body)
            data = open(os.path.join(cap, cbv[1]), 'rb').read() if cbv else b''
            if data and re.fullmatch(r'pass\d+(_post)?', name):
                u = struct.unpack('<68I', data[:272])
                got = [u[i:i + 2] for i in range(0, 68, 4)]
                if got != tensor_sizes((ow + 7) & ~7, (oh + 7) & ~7):
                    fail(f'{cap} {name}: tensor sizes {got}')
            if name == 'prepass':
                validate_prepass(cap, data, ow, oh, rw, rh, fail)
        init = re.search(r'COPYBUFFER r\d+\+0 <- r\d+\+0 size 131072 data=(data_\w+\.bin)', trace)
        blob = open(os.path.join(cap, init[1]), 'rb').read()
        if entry['init'] and entry['init'] != blob: fail(f'{cap}: initializer differs within {key}')
        entry['init'] = blob
    return sets
