#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer {
  public:
    Timer() {
        m_start = std::chrono::steady_clock::now();
    }

    template<typename T = std::chrono::milliseconds>
    double elapsed() {
        auto delta = peek();
        reset();
        return delta;
    }

    template<typename T = std::chrono::milliseconds>
    double peek() {
        auto end = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration<double, typename T::period>(end - m_start);
        return delta.count();
    }

    void reset() {
        m_start = std::chrono::steady_clock::now();
    }

  private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
};

#endif /* end of include guard: TIMER_HPP */
