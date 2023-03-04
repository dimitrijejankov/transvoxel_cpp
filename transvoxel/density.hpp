#pragma once

#include <array>
#include <cmath>
#include <limits>

template <typename T>
struct Density {
    static std::array<float, 3> to_normal(const T& a, const T& b, const T& c) {
        const float norm = std::sqrt(a * a + b * b + c * c);
        if (norm > std::numeric_limits<T>::epsilon()) {
            return {-a / norm, -b / norm, -c / norm};
        } else {
            return {0.0f, 0.0f, 0.0f};
        }
    }

    static T interp(const T& a, const T& b, const T& threshold) {
        if (std::abs(b - a) > std::numeric_limits<T>::epsilon()) {
            return (threshold - a) / (b - a);
        } else {
            return static_cast<T>(0.5f);
        }
    }

    static float as_f32(const T& value) {
        return static_cast<float>(value);
    }
};

template <>
struct Density<float> {
    static std::array<float, 3> to_normal(const float& a, const float& b, const float& c) {
        const float norm = std::sqrt(a * a + b * b + c * c);
        if (norm > std::numeric_limits<float>::epsilon()) {
            return {-a / norm, -b / norm, -c / norm};
        } else {
            return {0.0f, 0.0f, 0.0f};
        }
    }

    static float interp(const float& a, const float& b, const float& threshold) {
        if (std::abs(b - a) > std::numeric_limits<float>::epsilon()) {
            return (threshold - a) / (b - a);
        } else {
            return 0.5f;
        }
    }

    static float as_f32(const float& value) {
        return value;
    }
};

template <typename D, typename F>
struct ScalarField {
    /**
    Obtain the density at the given point in space
    */
    virtual D get_density(F x, F y, F z) const = 0;
    virtual ~ScalarField() = default;
};

template <typename D, typename F>
struct ScalarFieldForFn : public ScalarField<D, F> {
    ScalarFieldForFn(F&& f) : m_function(std::forward<F>(f)) {}

    D get_density(float x, float y, float z) override {
        return m_function(x, y, z);
    }

    F m_function;
};

template <typename D, typename F>
ScalarFieldForFn<D, F> make_scalar_field(F&& f) {
    return ScalarFieldForFn<D, F>(std::forward<F>(f));
}

template <typename C>
struct Coordinate {

    static C from_ratio(int a, unsigned int b) {
        return static_cast<C>(a) / static_cast<C>(b);
    }

    static C half(int a) {
        return 0.5f * static_cast<C>(a);
    }
};