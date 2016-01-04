#ifndef FILTERING_HPP
#define FILTERING_HPP

#include <trac0r/utils.hpp>

#include <cstdint>
#include <vector>
#include <numeric>

#include <glm/glm.hpp>

#include <iostream>

namespace trac0r {

// inline void bilateral_filter(const std::vector<uint32_t> &input, std::vector<uint32_t> &output,
//                              const float radius, const float sigma) {
// }
// }

// inline void gaussian_filter(const std::vector<uint32_t> &input, std::vector<uint32_t> &output,

inline void box_filter(const std::vector<uint32_t> &input, const uint32_t width,
                       const uint32_t height, std::vector<uint32_t> &output) {
    // clang-format off
    std::vector<float> kernel = {1.f, 1.f, 1.f,
                                 1.f, 2.f, 1.f,
                                 1.f, 1.f, 1.f};
    // clang-format on

    int kernel_width = glm::sqrt(kernel.size());
    int offset = kernel_width / 2;
    float weightsum = std::accumulate(kernel.cbegin(), kernel.cend(), 0.f);

#pragma omp parallel for collapse(2) schedule(dynamic, 1024)
    for (uint32_t x = 1; x < width - 1; x++) {
        for (uint32_t y = 1; y < height - 1; y++) {
            glm::vec4 new_color;
            for (int i = 0; i < kernel_width; i++) {
                for (int j = 0; j < kernel_width; j++) {
                    glm::vec4 color = unpack_color_argb_to_vec4(
                        input[(x + i - offset) + (y + j - offset) * width]);
                    new_color += color * kernel[i + j * kernel_width];
                }
            }
            new_color /= weightsum;
            new_color.a = 1.f;
            output[x + y * width] = pack_color_argb(new_color);
        }
    }
}
}

#endif /* end of include guard: FILTERING_HPP */
