#ifndef INTERSECTIONS_HPP
#define INTERSECTIONS_HPP

#include "aabb.hpp"
#include "ray.hpp"
#include "triangle.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace trac0r {
// From
// http://ftp.cg.cs.tu-bs.de/media/publications/fast-rayaxis-aligned-bounding-box-overlap-tests-using-ray-slopes.pdf
// Fast Ray/Axis-Aligned Bounding Box Overlap Tests using Ray Slopes by Martin Eisemann et al.
inline bool intersect_ray_aabb_broken2(const Ray &ray, const AABB &aabb) {
    if (AABB::is_null(aabb))
        return false;

    auto s_yx = ray.m_dir.x * ray.m_invdir.y;
    auto s_xy = ray.m_dir.y * ray.m_invdir.x;
    auto s_zy = ray.m_dir.y * ray.m_invdir.z;
    auto s_yz = ray.m_dir.z * ray.m_invdir.y;
    auto s_xz = ray.m_dir.x * ray.m_invdir.z;
    auto s_zx = ray.m_dir.z * ray.m_invdir.x;

    auto c_xy = ray.m_origin.y - s_xy * ray.m_origin.x;
    auto c_yx = ray.m_origin.x - s_yx * ray.m_origin.y;
    auto c_zy = ray.m_origin.y - s_zy * ray.m_origin.z;
    auto c_yz = ray.m_origin.z - s_yz * ray.m_origin.y;
    auto c_xz = ray.m_origin.z - s_xz * ray.m_origin.x;
    auto c_zx = ray.m_origin.x - s_zx * ray.m_origin.z;

    if ((ray.m_origin.x > AABB::max(aabb).x) || (ray.m_origin.y > AABB::max(aabb).y) ||
        (ray.m_origin.z > AABB::max(aabb).z) || (s_xy * AABB::max(aabb).x - AABB::min(aabb).y + c_xy < 0) ||
        (s_yx * AABB::max(aabb).y - AABB::min(aabb).x + c_yx < 0) ||
        (s_zy * AABB::max(aabb).z - AABB::min(aabb).y + c_zy < 0) ||
        (s_yz * AABB::max(aabb).y - AABB::min(aabb).z + c_yz < 0) ||
        (s_xz * AABB::max(aabb).x - AABB::min(aabb).z + c_xz < 0) ||
        (s_zx * AABB::max(aabb).z - AABB::min(aabb).x + c_zx < 0))
        return false;

    return true;
}

// From http://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
inline bool intersect_ray_aabb_broken1(const Ray &ray, const AABB &aabb) {
    double t1 = (AABB::min(aabb).x - ray.m_origin.x) * ray.m_invdir.x;
    double t2 = (AABB::max(aabb).x - ray.m_origin.x) * ray.m_invdir.x;

    double tmin = glm::min(t1, t2);
    double tmax = glm::max(t1, t2);

    for (int i = 1; i < 3; ++i) {
        t1 = (AABB::min(aabb)[i] - ray.m_origin[i]) * ray.m_invdir[i];
        t2 = (AABB::max(aabb)[i] - ray.m_origin[i]) * ray.m_invdir[i];

        tmin = glm::max(tmin, glm::min(t1, t2));
        tmax = glm::min(tmax, glm::max(t1, t2));
    }

    return tmax > glm::max(tmin, 0.0);
}

// From
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
inline bool intersect_ray_aabb(const Ray &ray, const AABB &aabb) {
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    glm::vec3 bounds[2];
    bounds[0] = AABB::min(aabb);
    bounds[1] = AABB::max(aabb);

    glm::i8vec3 sign;
    sign.x = (ray.m_invdir.x < 0);
    sign.y = (ray.m_invdir.y < 0);
    sign.z = (ray.m_invdir.z < 0);

    tmin = (bounds[sign.x].x - ray.m_origin.x) * ray.m_invdir.x;
    tmax = (bounds[1 - sign.x].x - ray.m_origin.x) * ray.m_invdir.x;
    tymin = (bounds[sign.y].y - ray.m_origin.y) * ray.m_invdir.y;
    tymax = (bounds[1 - sign.y].y - ray.m_origin.y) * ray.m_invdir.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[sign.z].z - ray.m_origin.z) * ray.m_invdir.z;
    tzmax = (bounds[1 - sign.z].z - ray.m_origin.z) * ray.m_invdir.z;

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
inline bool intersect_ray_triangle(const Ray &ray, const Triangle &triangle, float &dist) {
    // Calculate edges of triangle from v0.
    auto e0 = triangle.m_v2 - triangle.m_v1;
    auto e1 = triangle.m_v3 - triangle.m_v1;

    // Calculate determinant to check whether the ray is in the newly calculated plane made up from
    // e0 and e1.
    auto pvec = glm::cross(ray.m_dir, e1);
    auto det = glm::dot(e0, pvec);

    // Check whether determinant is close to 0. If that is the case, the ray is in the same plane as
    // the triangle itself which means that they can't collide. This effectively disables backface
    // culling for which we would instead only check whether det < epsilon.
    auto epsilon = std::numeric_limits<float>::epsilon();
    if (det > -epsilon && det < epsilon)
        return false;

    auto inv_det = 1.f / det;

    // Calculate distance from v1 to ray origin
    auto tvec = ray.m_origin - triangle.m_v1;

    // Calculate u parameter and test bound
    auto u = glm::dot(tvec, pvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (u < 0.f || u > 1.f)
        return false;

    // Prepare to test v parameter
    auto qvec = glm::cross(tvec, e0);

    // Calculate v parameter and test bound
    auto v = glm::dot(ray.m_dir, qvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (v < 0.f || u + v > 1.f)
        return false;

    auto t = glm::dot(e1, qvec) * inv_det;

    if (t > epsilon) {
        dist = t - epsilon;
        return true;
    }

    // If we end up here, there was no hit
    return false;
}
}

#endif /* end of include guard: INTERSECTIONS_HPP */
