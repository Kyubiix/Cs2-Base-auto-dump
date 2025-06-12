#pragma once

#include <cmath>
#include <numbers>
#include "../../ext/imgui/imgui.h"

// View Matrix Struct
struct view_matrix_t {
    float matrix[4][4];

    float* operator[](int index) {
        return matrix[index];
    }

    const float* operator[](int index) const {
        return matrix[index];
    }
};

// 3D Vector Struct
struct Vector3 {
    float x, y, z;

    constexpr Vector3(float x = 0.f, float y = 0.f, float z = 0.f) noexcept : x(x), y(y), z(z) {}

    // Arithmetic Operators
    constexpr Vector3 operator-(const Vector3& other) const noexcept {
        return Vector3{ x - other.x, y - other.y, z - other.z };
    }

    constexpr Vector3 operator+(const Vector3& other) const noexcept {
        return Vector3{ x + other.x, y + other.y, z + other.z };
    }

    constexpr Vector3 operator/(float factor) const noexcept {
        return Vector3{ x / factor, y / factor, z / factor };
    }

    constexpr Vector3 operator*(float factor) const noexcept {
        return Vector3{ x * factor, y * factor, z * factor };
    }

    // Utilities
    float Length() const noexcept {
        return std::sqrt(x * x + y * y + z * z);
    }

    float Length2D() const noexcept {
        return std::sqrt(x * x + y * y);
    }

    constexpr bool IsZero() const noexcept {
        return x == 0.f && y == 0.f && z == 0.f;
    }

    Vector3 w2s(const view_matrix_t& matrix) const {
        float _x = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z + matrix[0][3];
        float _y = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z + matrix[1][3];
        float w = matrix[3][0] * x + matrix[3][1] * y + matrix[3][2] * z + matrix[3][3];

        if (w < 0.01f) {
            return Vector3{ 0.f, 0.f, w }; // Not visible
        }

        float inv_w = 1.f / w;
        _x *= inv_w;
        _y *= inv_w;

        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        float screen_x = screenWidth * 0.5f + 0.5f * _x * screenWidth;
        float screen_y = screenHeight * 0.5f - 0.5f * _y * screenHeight;

        return Vector3{ screen_x, screen_y, w };
    }
};
