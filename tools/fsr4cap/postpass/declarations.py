# SPDX-License-Identifier: GPL-2.0-or-later
"""SPIR-V declarations and constants for postpass LDS rewriting."""

SLOT = {'%rw_history_color': 0, '%rw_mlsr_output_color': 3, '%rw_recurrent_0': 6}

DECL = [
    '%bb_u5 = OpConstant %uint 5', '%bb_u31 = OpConstant %uint 31',
    '%bb_u32 = OpConstant %uint 32', '%bb_u10 = OpConstant %uint 10',
    '%bb_u256 = OpConstant %uint 256', '%bb_u2 = OpConstant %uint 2',
    '%bb_u264 = OpConstant %uint 264', '%bb_u10240 = OpConstant %uint 10240',
] + [f'%bb_c{c} = OpConstant %uint {c}' for c in range(10)] + [
    '%bb_arr = OpTypeArray %float %bb_u10240',
    '%bb_ptr_arr = OpTypePointer Workgroup %bb_arr',
    '%bb_ptr_f = OpTypePointer Workgroup %float',
    '%bb_lds = OpVariable %bb_ptr_arr Workgroup',
] + [f'%bb_k{k} = OpConstant %uint {256 * k}' for k in range(4)]

PREAMBLE = [
    '%bb_wgx_p = OpAccessChain %_ptr_Input_uint %gl_WorkGroupID %uint_0',
    '%bb_wgx = OpLoad %uint %bb_wgx_p',
    '%bb_wgy_p = OpAccessChain %_ptr_Input_uint %gl_WorkGroupID %uint_1',
    '%bb_wgy = OpLoad %uint %bb_wgy_p',
    '%bb_lid_p = OpAccessChain %_ptr_Input_uint %gl_LocalInvocationID %uint_0',
    '%bb_lid = OpLoad %uint %bb_lid_p',
    '%bb_x0 = OpShiftLeftLogical %uint %bb_wgx %bb_u5',
    '%bb_y0 = OpShiftLeftLogical %uint %bb_wgy %bb_u5',
]


class IdGen:
    def __init__(self):
        self.n = 0

    def new(self, prefix):
        self.n += 1
        return f'%bb_{prefix}{self.n}'
