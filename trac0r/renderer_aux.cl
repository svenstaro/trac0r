#define EPSILON 0.00001

typedef struct PRNG {
    ulong m_seed[16];
    ulong m_p;
} PRNG;

typedef struct Camera {
    float3 m_pos;
    float3 m_dir;
    float3 m_world_up;
    float3 m_right;
    float3 m_up;
    float m_canvas_width;
    float m_canvas_height;
    float3 m_canvas_center_pos;
    float3 m_canvas_dir_x;
    float3 m_canvas_dir_y;
    float m_near_plane_dist;
    float m_far_plane_dist;
    int m_screen_width;
    int m_screen_height;
    float m_vertical_fov;
    float m_horizontal_fov;
} Camera;

typedef struct Material {
    float3 m_reflectance;
    float3 m_emittance;
} Material;

typedef struct Triangle {
    float3 m_v1;
    float3 m_v2;
    float3 m_v3;
    Material m_material;
    float3 m_normal;
    float3 m_centroid;
    float m_area;
} Triangle;

typedef struct FlatStructure {
    Triangle m_triangles[100];
    unsigned m_num_triangles;
} FlatStructure;

typedef struct Ray {
    float3 m_origin;
    float3 m_dir;
    float3 m_invdir;
} Ray;

typedef struct IntersectionInfo {
    bool m_has_intersected;
    float3 m_pos;
    float3 m_normal;
    Ray m_incoming_ray;
    Material m_material;
} IntersectionInfo;

typedef struct AABB {
    float3 m_min;
    float3 m_max;
} AABB;

// From http://xorshift.di.unimi.it/xorshift1024star.c
ulong xorshift1024star(__local PRNG *prng) {
    ulong s0 = prng->m_seed[prng->m_p];
    ulong s1 = prng->m_seed[prng->m_p = (prng->m_p + 1) & 15];
    s1 ^= s1 << 31; // a
    s1 ^= s1 >> 11; // b
    s0 ^= s0 >> 30; // c
    return (prng->m_seed[prng->m_p] = s0 ^ s1) * 1181783497276652981L;
}

float rand_range(__local PRNG *prng, const float min, const float max) {
    return min + ((float)xorshift1024star(prng)) / (float)(ULONG_MAX / (max - min));
}

bool AABB_is_null(const AABB *aabb) {
    bool min_null = length(aabb->m_min) == 0;
    bool max_null = length(aabb->m_max) == 0;
    return min_null && max_null;
}

float3 AABB_min(const AABB *aabb) {
    return aabb->m_min;
}

float3 AABB_max(const AABB *aabb) {
    return aabb->m_max;
}

float3 AABB_diagonal(const AABB *aabb) {
    return aabb->m_max - aabb->m_min;
}

float3 AABB_center(const AABB *aabb) {
    return aabb->m_min + (AABB_diagonal(aabb) * 0.5f);
}

// std_array<glm_vec3, 8> AABB_vertices(AABB &aabb) {
//     std_array<glm_vec3, 8> result;
//     result[0] = {aabb.m_min.x, aabb.m_min.y, aabb.m_min.z}; // lower left front
//     result[1] = {aabb.m_max.x, aabb.m_min.y, aabb.m_min.z}; // lower right front
//     result[2] = {aabb.m_min.x, aabb.m_max.y, aabb.m_min.z}; // upper left front
//     result[3] = {aabb.m_max.x, aabb.m_max.y, aabb.m_min.z}; // upper right front
//     result[4] = {aabb.m_min.x, aabb.m_min.y, aabb.m_max.z}; // lower left back
//     result[5] = {aabb.m_max.x, aabb.m_min.y, aabb.m_max.z}; // lower right back
//     result[6] = {aabb.m_min.x, aabb.m_max.y, aabb.m_max.z}; // upper left back
//     result[7] = {aabb.m_max.x, aabb.m_max.y, aabb.m_max.z}; // upper right back
//     return result;
// }

void AABB_extend(AABB *aabb, float3 point) {
    aabb->m_min = min(point, aabb->m_min);
    aabb->m_max = max(point, aabb->m_max);
}

bool AABB_overlaps(const AABB *first, const AABB *second) {
    return first->m_max.x > second->m_min.x && first->m_min.x < second->m_max.x &&
           first->m_max.y > second->m_min.y && first->m_min.y < second->m_max.y &&
           first->m_max.z > second->m_min.z && first->m_min.z < second->m_max.z;
}

void AABB_reset(AABB *aabb) {
    aabb->m_min = 0;
    aabb->m_max = 0;
}

float2 Camera_screenspace_to_camspace(__constant const Camera *camera, unsigned x, unsigned y) {
    float rel_x = -(x - camera->m_screen_width / 2.f) / camera->m_screen_width;
    float rel_y = -(y - camera->m_screen_height / 2.f) / camera->m_screen_height;
    return (float2)(rel_x, rel_y);
}

int2 Camera_camspace_to_screenspace(__constant const Camera *camera, int2 coords) {
    int screen_x = round(0.5f * (camera->m_screen_width - 2.f * camera->m_screen_width * coords.x));
    int screen_y =
        round(0.5f * (camera->m_screen_height - 2.f * camera->m_screen_height * coords.y));
    return (int2)(screen_x, screen_y);
}

float3 Camera_camspace_to_worldspace(__constant const Camera *camera, float2 rel_pos) {
    float3 worldspace = camera->m_canvas_center_pos + (rel_pos.x * camera->m_canvas_dir_x) +
                        (rel_pos.y * camera->m_canvas_dir_y);
    return worldspace;
}

// float2 Camera_worldspace_to_camspace(const Camera *camera, float3 world_pos_on_canvas) {
//     float3 canvas_center_to_point = world_pos_on_canvas - Camera_canvas_center_pos(camera);
//
//     // Manually calculate angle between the positive y-axis on the canvas and the world point
//     float3 ay = canvas_center_to_point;
//     float3 by = Camera_up(camera);
//     float3 cy = cross(ay, by);
//     float angle1y = atan(dot(ay, by), length(cy));
//
//     // Manually calculate angle between the positive x-axis on the canvas and the world point
//     float3 ax = canvas_center_to_point;
//     float3 bx = Camera_right(camera);
//     float3 cx = cross(ax, bx);
//     float angle1x = atan(length(cx), dot(ax, bx));
//
//     float angle2x = glm_pi<float>() - (glm_half_pi<float>() + angle1x);
//     float len = length(canvas_center_to_point);
//     float x = sin(angle2x) * len;
//     float y = sin(angle1y) * len;
//     float rel_x = -(2 * x) / Camera_canvas_width(camera);
//     float rel_y = -(2 * y) / Camera_canvas_height(camera);
//     return (float2)(rel_x, -rel_y);
// }

// glm_vec3 Camera_worldpoint_to_worldspace(const Camera *camera, glm_vec3 world_point) {
//     auto ray_to_cam = pos(camera) - world_point;
//     float dist = 0;
//     bool collided = glm_intersectRayPlane(world_point, glm_normalize(ray_to_cam),
//                                           canvas_center_pos(camera), dir(camera), dist);
//     if (collided) {
//         return world_point + glm_normalize(ray_to_cam) * dist;
//     } else {
//         return glm_vec3(0);
//     }
// }

// From
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool intersect_ray_aabb(const Ray *ray, const AABB *aabb) {
    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    float3 bounds[2];
    bounds[0] = AABB_min(aabb);
    bounds[1] = AABB_max(aabb);

    char3 sign;
    sign.x = (ray->m_invdir.x < 0);
    sign.y = (ray->m_invdir.y < 0);
    sign.z = (ray->m_invdir.z < 0);

    tmin = (bounds[sign.x].x - ray->m_origin.x) * ray->m_invdir.x;
    tmax = (bounds[1 - sign.x].x - ray->m_origin.x) * ray->m_invdir.x;
    tymin = (bounds[sign.y].y - ray->m_origin.y) * ray->m_invdir.y;
    tymax = (bounds[1 - sign.y].y - ray->m_origin.y) * ray->m_invdir.y;

    if ((tmin > tymax) || (tymin > tmax))
        return false;
    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[sign.z].z - ray->m_origin.z) * ray->m_invdir.z;
    tzmax = (bounds[1 - sign.z].z - ray->m_origin.z) * ray->m_invdir.z;

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
bool intersect_ray_triangle(const Ray *ray, __constant const Triangle *triangle, float *dist) {
    // Calculate edges of triangle from v0.
    float3 e0 = triangle->m_v2 - triangle->m_v1;
    float3 e1 = triangle->m_v3 - triangle->m_v1;

    // Calculate determinant to check whether the ray is in the newly calculated plane made up from
    // e0 and e1.
    float3 pvec = cross(ray->m_dir, e1);
    float det = dot(e0, pvec);

    // Check whether determinant is close to 0. If that is the case, the ray is in the same plane as
    // the triangle itself which means that they can't collide. This effectively disables backface
    // culling for which we would instead only check whether det < epsilon.
    if (det > -EPSILON && det < EPSILON)
        return false;

    float inv_det = 1.f / det;

    // Calculate distance from v1 to ray origin
    float3 tvec = ray->m_origin - triangle->m_v1;

    // Calculate u parameter and test bound
    float u = dot(tvec, pvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (u < 0.f || u > 1.f)
        return false;

    // Prepare to test v parameter
    float3 qvec = cross(tvec, e0);

    // Calculate v parameter and test bound
    float v = dot(ray->m_dir, qvec) * inv_det;

    // Check whether the intersection lies outside of the triangle
    if (v < 0.f || u + v > 1.f)
        return false;

    float t = dot(e1, qvec) * inv_det;

    if (t > EPSILON) {
        *dist = t - EPSILON;
        return true;
    }

    // If we end up here, there was no hit
    return false;
}

IntersectionInfo Scene_intersect(__constant FlatStructure *accel_struct, Ray *ray) {
    IntersectionInfo intersect_info;

    // Keep track of closest triangle
    float closest_dist = FLT_MAX;
    Triangle closest_triangle;
    // for (const auto &shape : FlatStructure::shapes(flatstruct)) {
    //     if (intersect_ray_aabb(ray, Shape::aabb(shape))) {
    //         for (auto &tri : Shape::triangles(shape)) {
    for (unsigned i = 0; i < accel_struct->m_num_triangles; i++) {
        float dist_to_intersect;
        __constant Triangle *tri = &(accel_struct->m_triangles[i]);
        bool intersected = intersect_ray_triangle(ray, tri, &dist_to_intersect);
        if (intersected) {
            // Find closest triangle
            if (dist_to_intersect < closest_dist) {
                closest_dist = dist_to_intersect;
                closest_triangle = *tri;

                intersect_info.m_has_intersected = true;
                intersect_info.m_pos = ray->m_origin + ray->m_dir * closest_dist;
                intersect_info.m_incoming_ray = *ray;
                intersect_info.m_normal = closest_triangle.m_normal;
                intersect_info.m_material = closest_triangle.m_material;
            }
        }
    }
    //     }
    // }

    return intersect_info;
}

__kernel void renderer_trace_pixel_color(__write_only __global float4 *output, const int width,
                                         const unsigned max_depth,
                                         __local PRNG *prng) { //, __constant Camera *camera,
    // __constant FlatStructure *flatstruct) {
    int x = get_global_id(0);
    int y = get_global_id(1);
    int index = y * width + x;

    // float2 rel_pos = Camera_screenspace_to_camspace(camera, x, y);
    // float3 world_pos = Camera_camspace_to_worldspace(camera, rel_pos);
    // float3 ray_dir = normalize(world_pos - camera->m_pos);
    //
    // float3 ret_color = (float3)(0);
    // Ray next_ray = {world_pos, ray_dir};
    // float3 brdf = (float3)(1);
    // for (unsigned depth = 0; depth < max_depth; depth++) {
    //     IntersectionInfo intersect_info = Scene_intersect(flatstruct, &next_ray);
    //     if (intersect_info.m_has_intersected) {
    //         // Get the local radiance only on first bounce
    //         float3 local_radiance;
    //         // if (depth == 0) {
    //         // auto ray = ray_pos - impact_pos;
    //         // float dist2 = glm_dot(ray, ray);
    //         // auto cos_area = glm_dot(-ray_dir, tri->m_normal) * tri->m_area;
    //         // auto solid_angle = cos_area / glm_max(dist2, 1e-6f);
    //         //
    //         // if (cos_area > 0.0)
    //         //     local_radiance = tri->m_emittance * solid_angle;
    //         //     local_radiance = intersect_info.m_material.m_emittance;
    //         // }
    //
    //         local_radiance = intersect_info.m_material.m_emittance;
    //
    //         // Emitter sample
    //         // TODO
    //         // glm_vec3 illumination;
    //
    //         float3 normal =
    //             intersect_info.m_normal *
    //             -sign(dot(intersect_info.m_normal, intersect_info.m_incoming_ray.m_dir));
    //
    //         // Find new random direction for diffuse reflection
    //         float u = 2.f * rand_range(prng, 0.f, 1.f);
    //         float v = 2.f * M_PI_F * rand_range(prng, 0.f, 1.f);
    //         float xx = sqrt(1 - u * u) * cos(v);
    //         float yy = sqrt(1 - u * u) * sin(v);
    //         float zz = u;
    //         float3 new_ray_dir = {xx, yy, zz};
    //         new_ray_dir = normalize(new_ray_dir);
    //         if (dot(new_ray_dir, normal) < 0) {
    //             new_ray_dir = -new_ray_dir;
    //         }
    //
    //         float cos_theta = dot(new_ray_dir, normal);
    //
    //         ret_color += brdf * local_radiance;
    //         brdf *= 2.f * intersect_info.m_material.m_reflectance * cos_theta;
    //
    //         // Make a new ray
    //         Ray new_ray = {intersect_info.m_pos, new_ray_dir};
    //         next_ray = new_ray;
    //
    //     } else {
    //         break;
    //     }
    // }
    //
    // output[index] = (float4)(ret_color.x, ret_color.y, ret_color.z, 1.f);
    output[index] = (float4)(.5f, .9f, .9f, 1.f);
}
