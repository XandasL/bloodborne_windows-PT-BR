# SPDX-License-Identifier: GPL-2.0-or-later
"""Dispatch rules, group counts, and tensor size validation for FSR 4.1.1."""

import re

FLAGS = [
    '--enable-shader-i8-dot', '--ssbo-uav', '--ssbo-srv', '--class-bindings',
    '--use-reflection-names', '--mixed-float-dot-product'
]
PREFIX = 'fsr4_model_v07_fp8_no_scale_'
SEQUENCE = ['spd', 'prepass', 'pass0_post'] + [f'pass{k}{s}' for k in range(1, 13) for s in ('', '_post')] + ['postpass', 'rcas']
LEVEL = {1: 1, 2: 1, 3: 2, 4: 2, 5: 2, 6: 3, 7: 3, 8: 3, 9: 3, 10: 2, 11: 2, 12: 1}


def ceil_div(a, b):
    return (a + b - 1) // b


def tensor_sizes(aw, ah):
    d = [1, 0, 1, 1, 2, 2, 2, 3, 3, 3, 2, 2, 1, 1, 0, 0, 0]
    return [(aw >> s, ah >> s) for s in d]


def shader_name(path):
    with open(path, 'rb') as f:
        data = f.read()
    for s in re.findall(rb'[A-Za-z_][A-Za-z0-9_]{5,80}', data):
        decoded = s.decode()
        if decoded.startswith(PREFIX):
            return decoded[len(PREFIX):]
        if decoded.startswith('fsr_rcas'):
            return 'rcas'
    return 'spd'


def expected_groups(name, rw, rh, ow, oh):
    aw, ah = (ow + 7) & ~7, (oh + 7) & ~7
    if name == 'spd':
        return (ceil_div(rw, 64), ceil_div(rh, 64), 1)
    if name in ('prepass', 'rcas'):
        return (ceil_div(aw, 16), ceil_div(ah, 16), 1)
    if name == 'postpass':
        return (ceil_div(aw, 32), ceil_div(ah, 32), 1)
    m = re.fullmatch(r'pass(\d+)', name)
    if m:
        lw, lh = aw >> LEVEL[int(m[1])], ah >> LEVEL[int(m[1])]
        return (ceil_div(lw, 64), lh, 1)
    return None
