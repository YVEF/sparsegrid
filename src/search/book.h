#ifndef SPARSEGRID_BOOK_H
#define SPARSEGRID_BOOK_H
#include <string_view>
#include <cstdint>
#include <vector>

namespace brd { class BoardState; struct Move; }
namespace common { struct Options; }
namespace search {
class Book {
public:
    explicit Book(const common::Options&) noexcept;
    bool probe(const brd::BoardState& state, brd::Move& move) const noexcept;
    void read(std::string_view path);

private:
struct SgPolyEntry {
    uint8_t move_from;
    uint8_t move_to;
    uint64_t key;
    uint16_t weight;
};
    std::vector<SgPolyEntry> m_entries;
    const common::Options& m_opts;
};

} // search

#endif //SPARSEGRID_BOOK_H
