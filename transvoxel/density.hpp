#pragma once

#include <array>
#include <cmath>
#include <limits>
#include <functional>

template <typename F>
struct Density {

    static bool inside(F value) {
        return value > 0.0f;
    }

    static std::array<float, 3> to_normal(const F& a, const F& b, const F& c) {
        const auto norm = std::sqrt(a * a + b * b + c * c);
        if (norm > std::numeric_limits<F>::epsilon()) {
            return {-a / norm, -b / norm, -c / norm};
        } else {
            return {0.0f, 0.0f, 0.0f};
        }
    }

    static float interp(const F& a, const F& b, const F& threshold) {
        if (std::abs(b - a) > std::numeric_limits<F>::epsilon()) {
            return (threshold - a) / (b - a);
        } else {
            return 0.5f;
        }
    }

    static F diff(F value, F other) {
        return value - other;
    }

    static constexpr float shrink_factor() {
        return 0.15f;
    }
};

template <typename C>
struct Coordinate {

    static C from_ratio(int a, unsigned int b) {
        return static_cast<C>(a) / static_cast<C>(b);
    }

    static C half(int a) {
        return 0.5f * static_cast<C>(a);
    }
};