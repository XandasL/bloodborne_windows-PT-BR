// SPDX-License-Identifier: GPL-2.0-or-later
#include "write_log_internal.h"
#include "write_log_scan.h"
#include <cstdio>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <ucontext.h>
#endif

extern "C" void bbgpu_dump_guest_writes(void* ucontext) {
    using namespace BbWriteLog;
    if (Mode() == 0 || !ucontext) return;

#ifdef _WIN32
    const auto* ctx = static_cast<const CONTEXT*>(ucontext);
    const std::uint64_t regs[] = {ctx->Rax, ctx->Rbx, ctx->Rcx, ctx->Rdx,
                                  ctx->Rsi, ctx->Rdi, ctx->R14, ctx->R15};
#else
    const auto* uc = static_cast<const ucontext_t*>(ucontext);
    const auto* g = uc->uc_mcontext.gregs;
    const std::uint64_t regs[] = {std::uint64_t(g[REG_RAX]), std::uint64_t(g[REG_RBX]),
                                  std::uint64_t(g[REG_RCX]), std::uint64_t(g[REG_RDX]),
                                  std::uint64_t(g[REG_RSI]), std::uint64_t(g[REG_RDI]),
                                  std::uint64_t(g[REG_R14]), std::uint64_t(g[REG_R15])};
#endif

    const char* names[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r14", "r15"};
    for (int i = 0; i < 8; ++i) {
        std::fprintf(stderr, "Write log: %s=%#llx\n", names[i], (unsigned long long)regs[i]);
    }

    DumpPatternHits();
    DumpBlockWrites(regs[0]);
    DumpMatchingWrites(regs[0], regs);
}
