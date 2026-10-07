# SPDX-License-Identifier: GPL-2.0-or-later
"""SPIR-V instruction stream transformer."""

import re
import sys
from .context import parse_context
from .declarations import DECL, PREAMBLE, SLOT, IdGen
from .flush import emit_flush


def rewrite_spirv(lines):
    main_i, merge, w2, h2, image_type, defs = parse_context(lines)
    id_gen = IdGen()
    new = id_gen.new
    out, stores, in_main, merge_seen, preamble = [], 0, False, False, list(PREAMBLE)
    for line in lines:
        s = line.strip()
        if s.startswith('%main = OpFunction'):
            out += ['               ' + d for d in DECL]; in_main = True; out.append(line); continue
        if in_main and re.match(r'%\w+ = OpLabel$', s) and not merge_seen and preamble:
            out.append(line); out += ['               ' + p for p in preamble]; preamble = []; continue
        m = re.match(r'OpImageWrite (%\w+) (%\w+) (%\w+)$', s)
        if in_main and m:
            var = defs[m[1]].split()[-1]
            coord = re.match(r'OpCompositeConstruct %v2uint (%\w+) (%\w+)$', defs[m[2]])
            if var not in SLOT or not coord: sys.exit(f'postpass_lds: unexpected store {s}')
            x, y = coord[1], coord[2]
            texel = defs[m[3]]
            if var == '%rw_recurrent_0':
                if not texel.startswith('OpFConvert %v4float'): sys.exit(f'postpass_lds: recurrent texel {texel}')
                comps = [new('r') for _ in range(4)]
                for c, v in enumerate(comps):
                    out.append(f'               {v} = OpCompositeExtract %float {m[3]} {c}')
            else:
                t = re.match(r'OpCompositeConstruct %v4float (%\w+) (%\w+) (%\w+) (%\w+)$', texel)
                if not t or t[4] != t[1]: sys.exit(f'postpass_lds: texel of {var} is not (x, y, z, x): {texel}')
                comps = [t[1], t[2], t[3]]
            lx, ly, row, pix, base = new('slx'), new('sly'), new('srow'), new('spix'), new('sbase')
            out += ['               ' + c for c in [
                f'{lx} = OpISub %uint {x} %bb_x0', f'{ly} = OpISub %uint {y} %bb_y0',
                f'{row} = OpShiftLeftLogical %uint {ly} %bb_u5', f'{pix} = OpIAdd %uint {row} {lx}',
                f'{base} = OpIMul %uint {pix} %bb_u10']]
            for c, v in enumerate(comps):
                e, p = new('se'), new('sp')
                out += ['               ' + q for q in [f'{e} = OpIAdd %uint {base} %bb_c{SLOT[var] + c}',
                                                        f'{p} = OpAccessChain %bb_ptr_f %bb_lds {e}', f'OpStore {p} {v}']]
            stores += 1; continue
        if in_main and s == f'{merge} = OpLabel': merge_seen = True
        if in_main and merge_seen and s == 'OpReturn':
            out += ['               ' + f for f in emit_flush(id_gen, w2, h2, image_type)]
        out.append(line)
    if stores != 12: sys.exit(f'postpass_lds: expected 12 stores, rewrote {stores}')
    return '\n'.join(out)
