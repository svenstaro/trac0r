#include "flat_structure.hpp"
#include "intersections.hpp"

#include <algorithm>
#include <memory>

namespace trac0r {

void FlatStructure::add_shape(FlatStructure &flatstruct, Shape &shape) {
    flatstruct.m_shapes.push_back(shape);
}

std::vector<Shape> &FlatStructure::shapes(FlatStructure &flatstruct) {
    return flatstruct.m_shapes;
}

const std::vector<Shape> &FlatStructure::shapes(const FlatStructure &flatstruct) {
    return flatstruct.m_shapes;
}

IntersectionInfo FlatStructure::intersect(const FlatStructure &flatstruct, const Ray &ray) {
    // TODO: We get like 2x better performance here if we loop over a flat structure of triangles
    // instead of looping over all shapes and for each shape over all triangles
    IntersectionInfo intersect_info;

    // Keep track of closest triangle
    float closest_dist = std::numeric_limits<float>::max();
    Triangle closest_triangle;
    for (const auto &shape : FlatStructure::shapes(flatstruct)) {
        if (intersect_ray_aabb(ray, Shape::aabb(shape))) {
            for (auto &tri : Shape::triangles(shape)) {
                float dist_to_intersect;
                bool intersected = intersect_ray_triangle(ray, tri, dist_to_intersect);
                if (intersected) {
                    // Find closest triangle
                    if (dist_to_intersect < closest_dist) {
                        closest_dist = dist_to_intersect;
                        closest_triangle = tri;

                        intersect_info.m_has_intersected = true;
                        intersect_info.m_pos = ray.m_origin + ray.m_dir * closest_dist;
                        intersect_info.m_incoming_ray = ray;

                        intersect_info.m_angle_between = glm::dot(
                            closest_triangle.m_normal, intersect_info.m_incoming_ray.m_dir);

                        intersect_info.m_normal = closest_triangle.m_normal;
                        intersect_info.m_material = closest_triangle.m_material;
                    }
                }
            }
        }
    }

    return intersect_info;
}

void FlatStructure::rebuild(FlatStructure &flatstruct, const Camera &camera) {
    (void)flatstruct;
    (void)camera;
    //     m_triangles.clear();
    //     for (auto &shape : m_shapes) {
    //         for (auto &tri : shape->triangles()) {
    //             m_triangles.push_back(*tri);
    //         }
    //     }
    //     // Sort by distance to camera
    //     // This is obviously broken but it works well enough for now
    //         std::sort(m_triangles.begin(), m_triangles.end(),
    //                   [&camera](const auto &tri1, const auto &tri2) {
    //                       return glm::distance(camera.pos(), tri1.m_centroid) <
    //                              glm::distance(camera.pos(), tri2.m_centroid);
    //                   });
}
}
