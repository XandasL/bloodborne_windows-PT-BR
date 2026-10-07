# SPDX-License-Identifier: GPL-2.0-or-later
"""SPIR-V context parser."""

import re
import sys


def parse_context(lines):
    def find(pat, start=0):
        for i in range(start, len(lines)):
            m = re.search(pat, lines[i])
            if m: return i, m
        sys.exit(f'postpass_lds: no match for {pat}')

    main_i, _ = find(r'^\s*%main = OpFunction ')
    sel_i, sel = find(r'OpSelectionMerge (%\w+) None', main_i)
    merge = sel[1]
    _, cond = find(r'OpBranchConditional (%\w+) ' + re.escape(merge) + r' (%\w+)', sel_i)
    _, lor = find(r'^\s*' + re.escape(cond[1]) + r' = OpLogicalOr %bool (%\w+) (%\w+)', main_i)
    _, gx = find(r'^\s*' + re.escape(lor[1]) + r' = OpUGreaterThanEqual %bool (%\w+) (%\w+)', main_i)
    _, gy = find(r'^\s*' + re.escape(lor[2]) + r' = OpUGreaterThanEqual %bool (%\w+) (%\w+)', main_i)
    w2, h2 = gx[2], gy[2]
    image_type, defs = {}, {}
    for line in lines[main_i:]:
        m = re.match(r'\s*(%\w+) = OpLoad (%\w+) (%rw_\w+)$', line)
        if m: image_type[m[3]] = m[2]
    for name in ('%rw_history_color', '%rw_mlsr_output_color', '%rw_recurrent_0'):
        if name not in image_type: sys.exit(f'postpass_lds: no store to {name}')
    for line in lines:
        m = re.match(r'\s*(%\w+) = (.*)$', line)
        if m: defs[m[1]] = m[2]
    return main_i, merge, w2, h2, image_type, defs
