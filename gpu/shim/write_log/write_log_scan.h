// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cstdint>

namespace BbWriteLog {
void DumpPatternHits();
void DumpBlockWrites(std::uint64_t block);
void DumpMatchingWrites(std::uint64_t block, const std::uint64_t regs[8]);
} // namespace BbWriteLog
