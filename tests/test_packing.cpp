#include "trac0r/utils.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    auto unpacked = glm::vec4{0.5, 0.5, 0.5, 0.5};
    for (int i = 0; i < 10000; ++i) {
        auto packed = trac0r::pack_color_argb(unpacked);
        unpacked = trac0r::unpack_color_argb_to_vec4(packed);
    }

    std::cout << glm::to_string(unpacked) << std::endl;

    return 0;
}
