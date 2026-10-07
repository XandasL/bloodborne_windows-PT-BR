# SPDX-License-Identifier: GPL-2.0-or-later
"""Postpass workgroup flush code generator."""


def emit_flush(id_gen, w2, h2, image_type):
    """Generate workgroup store flush instructions in contiguous row order."""
    new = id_gen.new
    code = ['OpControlBarrier %bb_u2 %bb_u2 %bb_u264']
    for k in range(4):
        i, lx, ly = new('i'), new('lx'), new('ly')
        px, py = new('px'), new('py')
        code += [
            f'{i} = OpIAdd %uint %bb_lid %bb_k{k}',
            f'{lx} = OpBitwiseAnd %uint {i} %bb_u31',
            f'{ly} = OpShiftRightLogical %uint {i} %bb_u5',
            f'{px} = OpIAdd %uint %bb_x0 {lx}',
            f'{py} = OpIAdd %uint %bb_y0 {ly}',
        ]
        tx, ty = new('tx'), new('ty')
        cx, cy, c = new('cx'), new('cy'), new('c')
        code += [
            f'{tx} = OpShiftRightLogical %uint {px} %uint_1',
            f'{ty} = OpShiftRightLogical %uint {py} %uint_1',
            f'{cx} = OpULessThan %bool {tx} {w2}',
            f'{cy} = OpULessThan %bool {ty} {h2}',
            f'{c} = OpLogicalAnd %bool {cx} {cy}',
        ]
        then, done = f'%bb_then{k}', f'%bb_done{k}'
        code += [f'OpSelectionMerge {done} None', f'OpBranchConditional {c} {then} {done}', f'{then} = OpLabel']
        base = new('base')
        code.append(f'{base} = OpIMul %uint {i} %bb_u10')
        vals = []
        for comp in range(10):
            e, p, v = new('e'), new('p'), new('v')
            code += [f'{e} = OpIAdd %uint {base} %bb_c{comp}', f'{p} = OpAccessChain %bb_ptr_f %bb_lds {e}',
                     f'{v} = OpLoad %float {p}']
            vals.append(v)
        coord = new('coord')
        code.append(f'{coord} = OpCompositeConstruct %v2uint {px} {py}')
        texels = {
            '%rw_history_color': [vals[0], vals[1], vals[2], vals[0]],
            '%rw_mlsr_output_color': [vals[3], vals[4], vals[5], vals[3]],
            '%rw_recurrent_0': vals[6:10],
        }
        for name, comps in texels.items():
            img, tex = new('img'), new('tex')
            code += [f'{img} = OpLoad {image_type[name]} {name}',
                     f'{tex} = OpCompositeConstruct %v4float ' + ' '.join(comps),
                     f'OpImageWrite {img} {coord} {tex}']
        code += [f'OpBranch {done}', f'{done} = OpLabel']
    return [c for c in code if c]
