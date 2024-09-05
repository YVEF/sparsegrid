#ifndef SPARSEGRID_POLYGLOT_H
#define SPARSEGRID_POLYGLOT_H
#include <cstdint>

namespace brd { class BoardState; }
namespace common { struct Options; }
namespace adapters::polyglot {

uint64_t makeKey(const brd::BoardState&) noexcept;

} // namespace adapters::polyglot

#endif //SPARSEGRID_POLYGLOT_H
