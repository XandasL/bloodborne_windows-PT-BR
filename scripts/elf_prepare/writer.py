# SPDX-License-Identifier: GPL-2.0-or-later
"""Prepared boot binary and analysis report output writers."""

import collections
import hashlib
import json
from pathlib import Path
import struct
from .sfo_parser import sfo

KNOWN_CANDIDATES = (
    '_init_env', 'atexit', 'exit', '_exit', '__cxa_atexit', '__cxa_finalize',
    '__cxa_guard_acquire', '__cxa_guard_release', '__cxa_guard_abort',
    '__stack_chk_guard', '__stack_chk_fail', 'malloc', 'free',
    'memcpy', 'memmove', 'memset', 'memcmp', 'strlen', 'printf',
    '_ZNSt8ios_base4InitC1Ev',
    'sceKernelLoadStartModule', 'sceVideoOutOpen', 'sceKernelCreateSema',
    'sceKernelWaitSema', 'sceKernelPollSema', 'sceKernelSignalSema',
    'sceKernelCancelSema', 'sceKernelDeleteSema', 'sceAppContentInitialize',
    'sceAppContentAppParamGetInt', 'sceAppContentGetAddcontInfoList',
    'pthread_cond_init', 'sceSysmoduleLoadModule', 'pthread_mutexattr_init',
    'gettimeofday', 'scePthreadMutexattrInit', 'scePthreadMutexattrSettype',
    'scePthreadMutexattrDestroy', 'scePthreadMutexInit', 'scePthreadMutexLock',
    'scePthreadMutexTrylock', 'scePthreadMutexUnlock', 'scePthreadMutexDestroy',
    'scePthreadRwlockInit', 'scePthreadRwlockDestroy', 'scePthreadRwlockRdlock',
    'scePthreadRwlockWrlock', 'scePthreadRwlockTryrdlock',
    'scePthreadRwlockTrywrlock', 'scePthreadRwlockUnlock',
    'scePthreadRwlockTimedrdlock', 'scePthreadRwlockTimedwrlock',
    'sceKernelGetDirectMemorySize', 'sceKernelAllocateDirectMemory',
    'sceKernelMapDirectMemory', 'sceKernelReleaseDirectMemory', 'sceKernelMunmap'
)


def write_boot_bin(out, size, entry, loads, relocs, names, init_ret, image):
    """Write compact BBPROBE2 binary."""
    with (out / 'boot.bin').open('wb') as f:
        f.write(struct.pack('<8sQQQQQQ', b'BBPROBE2', size, entry, len(loads),
                            len(relocs), len(names), int(init_ret)))
        for p in loads:
            f.write(struct.pack('<QQQ', p['vaddr'], p['memsz'], p['flags']))
        for name in names:
            encoded = name.encode()
            if len(encoded) >= 128:
                raise ValueError('import name too long')
            f.write(encoded.ljust(128, b'\0'))
        for relocation in relocs:
            f.write(struct.pack('<QQqq', *relocation))
        f.write(image)


def write_analysis(out, game, source, header, size, ph, segments, missing,
                   dyn, strings_fn, counts, names, known, libc_ev):
    """Generate and write analysis.json report."""
    res, total_bytes = collections.Counter(), 0
    for path in (game / 'dvdroot_ps4').rglob('*'):
        if path.is_file():
            res[path.relative_to(game / 'dvdroot_ps4').parts[0]] += 1
            total_bytes += path.stat().st_size
    report = dict(
        source_sha256=hashlib.sha256(source).hexdigest(), source_bytes=len(source),
        sfo=sfo((game / 'sce_sys/param.sfo').read_bytes()), entry=hex(header[4]),
        image_bytes=size, program_headers=ph, self_segments=segments,
        unavailable_metadata_headers=missing,
        needed=[strings_fn(v) for t, v in dyn if t == 1],
        relocation_counts=dict(counts), import_count=len(names), imports=names,
        import_name_hints={n: known[n.split('#')[0]] for n in names if n.split('#')[0] in known},
        libc_evidence=libc_ev,
        bundled_modules=sorted(p.name for p in (game / 'sce_module').iterdir()),
        resources=dict(res), resource_bytes=total_bytes,
        status='Prepared only; execution and Vulkan are tested separately.'
    )
    (out / 'analysis.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    return report
