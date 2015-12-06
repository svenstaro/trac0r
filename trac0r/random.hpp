#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdint>
#include <chrono>
#include <array>
#include <type_traits>

#include <glm/glm.hpp>

namespace trac0r {

// From http://xorshift.di.unimi.it/xorshift64star.c
inline uint64_t xorshift64star(uint64_t x) {
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    return x * 2685821657736338717LL;
}

// From http://xorshift.di.unimi.it/xorshift1024star.c
inline int64_t xorshift1024star(uint64_t &p, std::array<uint64_t, 16> &s) {
    uint64_t s0 = s[p];
    uint64_t s1 = s[p = (p + 1) & 15];
    s1 ^= s1 << 31; // a
    s1 ^= s1 >> 11; // b
    s0 ^= s0 >> 30; // c
    return (s[p] = s0 ^ s1) * 1181783497276652981LL;
}

class PRNG {
  public:
    PRNG() {
        // Generate an initial seed based on time
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

        // Then use the result of another PRNG as our actual seed
        for (auto i = 0; i < 16; i++)
            m_seed[i] = xorshift64star(ns);
    }

    uint64_t next() {
        return xorshift1024star(m_p, m_seed);
    }

  private:
    std::array<uint64_t, 16> m_seed;
    uint64_t m_p = 0;
};

template <typename T>
std::enable_if_t<std::is_integral<T>::value, T> inline rand_range(const T min, const T max) {
    static thread_local PRNG generator;
    return generator.next() % max + min;
}

template <typename T>
std::enable_if_t<std::is_floating_point<T>::value, T> inline rand_range(const T min, const T max) {
    static thread_local PRNG generator;
    return min +
           static_cast<T>(generator.next()) /
               (static_cast<T>(std::numeric_limits<uint64_t>::max() / (max - min)));
}

inline glm::vec3 uniform_sample_sphere() {
    static thread_local PRNG generator;
    glm::vec3 rand_vec =
        glm::vec3(rand_range(-1.f, 1.f), rand_range(-1.f, 1.f), rand_range(-1.f, 1.f));
    return normalize(rand_vec);
}
}

#endif /* end of include guard: RANDOM_HPP */
