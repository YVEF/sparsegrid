#ifndef INCLUDE_UCI_FEN_H_
#define INCLUDE_UCI_FEN_H_


#include <cstddef>
#include <string_view>

namespace brd { class BoardState; }

namespace uci {
/*
 * the FEN notation parser
 */
class Fen {
public:
    std::size_t apply(std::string_view input, brd::BoardState& state);
    std::string str(const brd::BoardState& state) const noexcept;
};

} // namespace uci


#endif  // INCLUDE_UCI_FEN_H_
