#ifndef INTERSECTIONS_HPP
#define INTERSECTIONS_HPP

#include "aabb.hpp"

#include <glm/glm.hpp>

#include <cppformat/format.h>
#include <glm/gtx/string_cast.hpp>

namespace trac0r {
// From
// http://ftp.cg.cs.tu-bs.de/media/publications/fast-rayaxis-aligned-bounding-box-overlap-tests-using-ray-slopes.pdf
// Fast Ray/Axis-Aligned Bounding Box Overlap Tests using Ray Slopes by Martin Eisemann et al.
// inline bool intersect_ray_aabb(const glm::vec3 &ray_pos, const glm::vec3 &ray_dir,
//                                const AABB &aabb) {
//     if (aabb.is_null())
//         return false;
//
//     glm::vec3 ray_dir_inv = 1.f / ray_dir;
//     auto s_yx = ray_dir.x * ray_dir_inv.y;
//     auto s_xy = ray_dir.y * ray_dir_inv.x;
//     auto s_zy = ray_dir.y * ray_dir_inv.z;
//     auto s_yz = ray_dir.z * ray_dir_inv.y;
//     auto s_xz = ray_dir.x * ray_dir_inv.z;
//     auto s_zx = ray_dir.z * ray_dir_inv.x;
//
//     auto c_xy = ray_pos.y - s_xy * ray_pos.x;
//     auto c_yx = ray_pos.x - s_yx * ray_pos.y;
//     auto c_zy = ray_pos.y - s_zy * ray_pos.z;
//     auto c_yz = ray_pos.z - s_yz * ray_pos.y;
//     auto c_xz = ray_pos.z - s_xz * ray_pos.x;
//     auto c_zx = ray_pos.x - s_zx * ray_pos.z;
//
//     if ((ray_pos.x > aabb.max().x) || (ray_pos.y > aabb.max().y) || (ray_pos.z > aabb.max().z) ||
//         (s_xy * aabb.max().x - aabb.min().y + c_xy < 0) ||
//         (s_yx * aabb.max().y - aabb.min().x + c_yx < 0) ||
//         (s_zy * aabb.max().z - aabb.min().y + c_zy < 0) ||
//         (s_yz * aabb.max().y - aabb.min().z + c_yz < 0) ||
//         (s_xz * aabb.max().x - aabb.min().z + c_xz < 0) ||
//         (s_zx * aabb.max().z - aabb.min().x + c_zx < 0))
//         return false;
//
//     return true;
// }

// inline bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir,
//                                const AABB &aabb) {
//     glm::vec3 dir_inv = 1.f / dir;
//
//     double t1 = (aabb.min().x - origin.x) * dir.x;
//     double t2 = (aabb.max().x - origin.x) * dir.x;
//
//     double tmin = glm::min(t1, t2);
//     double tmax = glm::max(t1, t2);
//
//     for (int i = 1; i < 3; ++i) {
//         t1 = (aabb.min()[i] - origin[i]) * dir_inv[i];
//         t2 = (aabb.max()[i] - origin[i]) * dir_inv[i];
//
//         tmin = glm::max(tmin, glm::min(t1, t2));
//         tmax = glm::min(tmax, glm::max(t1, t2));
//     }
//
//     return tmax > glm::max(tmin, 0.0);
// }

inline bool intersect_ray_aabb(const glm::vec3 &origin, const glm::vec3 &dir, const AABB &aabb) {
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    glm::vec3 bounds[2];
    bounds[0] = aabb.min();
    bounds[1] = aabb.max();

    glm::vec3 invdir = 1.f / dir;
    glm::i8vec3 sign;

    sign.x = (invdir.x < 0);
    sign.y = (invdir.y < 0);
    sign.z = (invdir.z < 0);

    tmin = (bounds[sign.x].x - origin.x) * invdir.x;
    tmax = (bounds[1 - sign.x].x - origin.x) * invdir.x;
    tymin = (bounds[sign.y].y - origin.y) * invdir.y;
    tymax = (bounds[1 - sign.y].y - origin.y) * invdir.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[sign.z].z - origin.z) * invdir.z;
    tzmax = (bounds[1 - sign.z].z - origin.z) * invdir.z;

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    return true;
}

// Möller-Trumbore intersection algorithm
// (see https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm)
inline bool intersect_ray_triangle(const glm::vec3 &origin, const glm::vec3 &dir,
                                   const glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2, float &dist) {
    // Calculate edges of triangle from v0.
    auto e0 = v1 - v0;
    auto e1 = v2 - v0;

    // Calculate determinant to check whether the ray is in the newly calculated plane made up from
    // e0 and e1.
    auto pvec = glm::cross(dir, e1);
    auto det = glm::dot(e0, pvec);

    // Check whether determinant is close to 0. If that is the case, the ray is in the same plane as
    // the triangle itself which means that they can't collide. This effectively disables backface
    // culling for which we would instead only check whether det < epsilon.
    auto epsilon = std::numeric_limits<float>::epsilon();
    if (det > -epsilon && det < epsilon)
        return false;

    auto inv_det = 1.f / det;

    // Calculate distance from v0 to ray origin
    auto tvec = origin - v0;

    // Calculate u parameter and test bound
    auto u = glm::dot(tvec, pvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (u < 0.f || u > 1.f)
        return false;

    // Prepare to test v parameter
    auto qvec = glm::cross(tvec, e0);

    // Calculate v parameter and test bound
    auto v = glm::dot(dir, qvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (v < 0.f || u + v > 1.f)
        return false;

    auto t = glm::dot(e1, qvec) * inv_det;

    if (t > epsilon) {
        dist = t;
        return true;
    }

    // If we end up here, there was no hit
    return false;
}
}

#endif /* end of include guard: INTERSECTIONS_HPP */
