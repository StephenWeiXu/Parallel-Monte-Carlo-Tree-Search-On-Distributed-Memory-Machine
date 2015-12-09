#ifndef MERSENNE_H
#define MERSENNE_H

#include <cassert>
#include <cstdint>
#include <iostream>

class MersenneTwister
{
    int m_ix;

    void Regenerate();

public:
    MersenneTwister();

    uint32_t Rand();
    uint64_t Rand64();
};

// Specialized templates to compute powers of two at compile time
// (template metaprogramming)
template<int NN>
struct Pow2
{
    enum { value = 2 * Pow2<NN - 1>::value };
};

template<>
struct Pow2<0>
{
    enum { value = 1 };
};

template<int NN>
struct Pow2Minus1
{
    enum { value = Pow2<NN - 1>::value - 1 + Pow2<NN - 1>::value };
};

#endif

