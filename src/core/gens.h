#ifndef INCLUDE_CORE_GENS_H_
#define INCLUDE_CORE_GENS_H_
#include <array>
#include <cstdint>
#include <type_traits>

namespace gen {
template<uint64_t V, uint64_t E>
constexpr
std::enable_if_t<(E==0), uint64_t> cpow_rec()
{ return 0; }

template<uint64_t V, uint64_t E>
constexpr
std::enable_if_t<(E>0), uint64_t> cpow_rec()
{ return V + cpow_rec<V, E-1>(); }

template<uint64_t V, uint64_t E>
constexpr
uint64_t cpow() {
    if constexpr(E == 0) { return 1; }
    return cpow_rec<V, E>();
}

template<uint64_t Seed, std::size_t Size, uint64_t SuplSeed = 11, typename T = void>
struct sequence_generator {
    constexpr std::array<uint64_t, Size> operator()() {
        sequence_generator<Seed, 500, SuplSeed> gen1{};
        constexpr auto seq1 = gen1();
        sequence_generator<Seed+SuplSeed, (Size-500), (SuplSeed+11)%UINT64_MAX> gen2{};
        constexpr auto seq2 = gen2();

        std::array<uint64_t, Size> result{};
        std::size_t i=0;
        for(std::size_t j=0; j<seq1.size(); j++)
            result[i++] = seq1[j];
        for(std::size_t j=0; j<seq2.size(); j++)
            result[i++] = seq2[j];
        
        return result;
    }
};

template<uint64_t Seed, std::size_t Size, uint64_t SuplSeed>
struct sequence_generator<Seed, Size, SuplSeed, std::enable_if_t<(Size<=500)>> {
    constexpr std::array<uint64_t, Size> operator()() 
    { return generate<>(); }

    template<std::size_t Idx=0>
    constexpr auto generate()
    {
        auto nmbr = std::array<uint64_t, Size>{};
        fill_sequence<Seed, Idx>(nmbr);
        return nmbr;
    }

    template<uint64_t TSeed, std::size_t Idx>
    constexpr 
    void fill_sequence(std::array<uint64_t, Size> &nmbr)
    {
        if constexpr(Idx < Size)
        {
            constexpr uint64_t new_seed = (((m_a * TSeed)%m_m) + m_c) % m_m;
            nmbr[Idx] = new_seed;
            static_assert(new_seed);
            fill_sequence<new_seed, Idx+1>(nmbr);
        }
    }
private:
    static constexpr uint64_t m_a = 1103515245u;
    static constexpr uint64_t m_c = 12345u;
    static constexpr uint64_t m_m = 18446744073709551557u;
};

struct rand {
    template<std::size_t Size, uint64_t Seed=11>
    constexpr auto gen_sequence_u64()
    {
        sequence_generator<Seed, Size> gen{};
        return gen();
    }
};

static constexpr uint64_t aaa = 95121u;
static constexpr uint64_t bbb = 73129u;
static constexpr uint64_t ccc = 18446744073709551557u;

uint64_t delay();



} // namespace gen

#endif  // INCLUDE_CORE_GENS_H_
