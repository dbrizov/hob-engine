#pragma once

#include <array>

namespace hob {
    // Column-major 4x4 matrix (translation in elements 12,13,14), matching the layout the
    // renderer uploads to HLSL cbuffers. Used for GPU clip-space / projection matrices;
    struct Matrix4x4 {
        std::array<float, 16> m{};

        const float* data() const { return m.data(); }
        float* data() { return m.data(); }

        // Size of the matrix in bytes — handy for GPU uniform pushes.
        static constexpr uint32_t byte_size() { return static_cast<uint32_t>(16 * sizeof(float)); }

        static Matrix4x4 identity() {
            Matrix4x4 out;
            out.m[0] = 1.0f;
            out.m[5] = 1.0f;
            out.m[10] = 1.0f;
            out.m[15] = 1.0f;
            return out;
        }

        // Orthographic projection mapping logical pixels x:[0,w] y:[0,h] -> NDC [-1,+1], y-down.
        // Matches the offscreen target's NDC y convention.
        static Matrix4x4 ortho_top_left(float w, float h) {
            Matrix4x4 out;
            out.m[0] = 2.0f / w;
            out.m[5] = 2.0f / h;
            out.m[10] = 1.0f;
            out.m[12] = -1.0f;
            out.m[13] = -1.0f;
            out.m[15] = 1.0f;
            return out;
        }

        // Same input range as ortho_top_left, but maps y:[0,h] -> [+1,-1].
        // Used for passes that target the swapchain directly (overlay/debug),
        // whose NDC y convention is opposite the offscreen target's.
        static Matrix4x4 ortho_top_left_y_flipped(float w, float h) {
            Matrix4x4 out;
            out.m[0] = 2.0f / w;
            out.m[5] = -2.0f / h;
            out.m[10] = 1.0f;
            out.m[12] = -1.0f;
            out.m[13] = 1.0f;
            out.m[15] = 1.0f;
            return out;
        }

        // Column-major matrix product (result = a * b): applies b first, then a.
        static Matrix4x4 multiply(const Matrix4x4& a, const Matrix4x4& b) {
            Matrix4x4 out;
            for (int col = 0; col < 4; ++col) {
                for (int row = 0; row < 4; ++row) {
                    float sum = 0.0f;
                    for (int k = 0; k < 4; ++k) {
                        sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                    }
                    out.m[col * 4 + row] = sum;
                }
            }

            return out;
        }
    };

    inline Matrix4x4 operator*(const Matrix4x4& a, const Matrix4x4& b) {
        return Matrix4x4::multiply(a, b);
    }
}
