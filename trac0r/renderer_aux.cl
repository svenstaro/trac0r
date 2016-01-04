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
    float2 m_pixel_size;
    float m_vertical_fov;
    float m_horizontal_fov;
} Camera;

typedef struct Material {
    uchar m_type;
    float3 m_color;
    float m_roughness;
    float m_ior;
    float m_emittance;
} Material;

typedef struct AABB {
    float3 m_min;
    float3 m_max;
} AABB;

typedef struct Shape {
    AABB m_aabb;
    uint m_triangle_index_start;
    uint m_triangle_index_end;
} Shape;

typedef struct Triangle {
    float3 m_v1;
    float3 m_v2;
    float3 m_v3;
    Material m_material;
    float3 m_normal;
    float3 m_centroid;
    float m_area;
} Triangle;

// typedef struct FlatStructure {
//     Triangle *m_triangles;
//     unsigned m_num_triangles;
// } FlatStructure;

typedef struct Ray {
    float3 m_origin;
    float3 m_dir;
    float3 m_invdir;
} Ray;

typedef struct IntersectionInfo {
    bool m_has_intersected;
    float3 m_pos;
    float3 m_normal;
    float m_angle_between;
    Ray m_incoming_ray;
    Material m_material;
} IntersectionInfo;

// From http://xorshift.di.unimi.it/xorshift1024star.c
inline ulong xorshift1024star(__global PRNG *prng) {
    ulong s0 = prng->m_seed[prng->m_p];
    ulong s1 = prng->m_seed[prng->m_p = (prng->m_p + 1) & 15];
    s1 ^= s1 << 31; // a
    s1 ^= s1 >> 11; // b
    s0 ^= s0 >> 30; // c
    return (prng->m_seed[prng->m_p] = s0 ^ s1) * 1181783497276652981L * get_global_id(0) *
           get_global_id(1);
}

inline Ray ray_construct(float3 origin, float3 direction) {
    Ray new_ray;
    new_ray.m_origin = origin;
    new_ray.m_dir = direction;
    new_ray.m_invdir = 1.f / direction;
    return new_ray;
}

inline float rand_range(__global PRNG *prng, const float min, const float max) {
    return min + ((float)xorshift1024star(prng)) / (float)(ULONG_MAX / (max - min));
}

inline float3 uniform_sample_sphere(__global PRNG *prng) {
    float3 rand_vec = (float3)(rand_range(prng, -1.f, 1.f), rand_range(prng, -1.f, 1.f),
                               rand_range(prng, -1.f, 1.f));
    return fast_normalize(rand_vec);
}

inline float3 oriented_oriented_hemisphere_sample(__global PRNG *prng, const float3 dir) {
    float3 v = uniform_sample_sphere(prng);
    return v * sign(dot(v, dir));
}

inline float3 ortho(float3 v) {
    // Awesome branchless function for finding an orthogonal vector in 3D space by
    // http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
    //
    // Their "boring" branching is commented here for completeness:
    // return glm::abs(v.x) > glm::abs(v.z) ? glm::vec3(-v.y, v.x, 0.0) : glm::vec3(0.0, -v.z, v.y);

    float flo; // We don't really care about the floor but fract needs it
    float k = fract(fabs(v.x) + 0.5f, &flo);
    return (float3)(-v.y, v.x - k * v.z, k * v.y);
}

inline float3 sample_hemisphere(__global PRNG *prng, float3 dir, float power, float angle) {
    // Code adapted from Mikael Hvidtfeldt Christensen's resource
    // at http://blog.hvidtfeldts.net/index.php/2015/01/path-tracing-3d-fractals/
    // Thanks!

    float3 o1 = fast_normalize(ortho(dir));
    float3 o2 = fast_normalize(cross(dir, o1));
    float2 r = (float2)(rand_range(prng, 0.f, 1.f), rand_range(prng, native_cos(angle), 1.f));
    r.x = r.x * M_PI_F * 2.f;
    r.y = native_powr(r.y, 1.f / (power + 1.f));
    float oneminus = sqrt(1.f - r.y * r.y);
    return native_cos(r.x) * oneminus * o1 + native_sin(r.x) * oneminus * o2 + r.y * dir;
}

inline float3 oriented_cosine_weighted_hemisphere_sample(__global PRNG *prng, float3 dir) {
    return sample_hemisphere(prng, dir, 1.f, M_PI_2_F);
}

inline float3 oriented_cosine_weighted_cone_sample(__global PRNG *prng, float3 dir, float angle) {
    return sample_hemisphere(prng, dir, 1.f, angle);
}

inline float3 reflect(float3 incident, float3 normal) {
    return incident - 2.f * normal * dot(normal, incident);
}

inline float3 refract(float3 incident, float3 normal, float eta) {
    float k = 1.f - eta * eta * (1.f - dot(normal, incident) * dot(normal, incident));
    if (k < 0.f)
        return (float3)(0.f);
    else
        return eta * incident - (eta * dot(normal, incident) + native_sqrt(k)) * normal;
}

inline bool AABB_is_null(const AABB *aabb) {
    bool min_null = fast_length(aabb->m_min) == 0;
    bool max_null = fast_length(aabb->m_max) == 0;
    return min_null && max_null;
}

inline float3 AABB_min(__global AABB *aabb) {
    return aabb->m_min;
}

inline float3 AABB_max(__global AABB *aabb) {
    return aabb->m_max;
}

inline float3 AABB_diagonal(const AABB *aabb) {
    return aabb->m_max - aabb->m_min;
}

inline float3 AABB_center(const AABB *aabb) {
    return aabb->m_min + (AABB_diagonal(aabb) * 0.5f);
}

inline void AABB_extend(AABB *aabb, float3 point) {
    aabb->m_min = min(point, aabb->m_min);
    aabb->m_max = max(point, aabb->m_max);
}

inline bool AABB_overlaps(const AABB *first, const AABB *second) {
    return first->m_max.x > second->m_min.x && first->m_min.x < second->m_max.x &&
           first->m_max.y > second->m_min.y && first->m_min.y < second->m_max.y &&
           first->m_max.z > second->m_min.z && first->m_min.z < second->m_max.z;
}

inline void AABB_reset(AABB *aabb) {
    aabb->m_min = 0;
    aabb->m_max = 0;
}

inline float2 Camera_screenspace_to_camspace(__constant const Camera *camera, unsigned x,
                                             unsigned y) {
    float rel_x = -(x - camera->m_screen_width / 2.f) / camera->m_screen_width;
    float rel_y = -(y - camera->m_screen_height / 2.f) / camera->m_screen_height;
    return (float2)(rel_x, rel_y);
}

inline int2 Camera_camspace_to_screenspace(__constant const Camera *camera, int2 coords) {
    int screen_x = round(0.5f * (camera->m_screen_width - 2.f * camera->m_screen_width * coords.x));
    int screen_y =
        round(0.5f * (camera->m_screen_height - 2.f * camera->m_screen_height * coords.y));
    return (int2)(screen_x, screen_y);
}

inline float3 Camera_camspace_to_worldspace(__constant const Camera *camera, float2 rel_pos) {
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

inline Ray Camera_pixel_to_ray(__global PRNG *prng, __constant Camera *camera, uint x, uint y) {
    float2 rel_pos = Camera_screenspace_to_camspace(camera, x, y);

    // Subpixel sampling / antialiasing
    float2 jitter = {rand_range(prng, -camera->m_pixel_size.x / 2.f, camera->m_pixel_size.x / 2.f),
                     rand_range(prng, -camera->m_pixel_size.y / 2.f, camera->m_pixel_size.y / 2.f)};
    rel_pos += jitter;

    float3 world_pos = Camera_camspace_to_worldspace(camera, rel_pos);
    float3 ray_dir = fast_normalize(world_pos - camera->m_pos);

    return ray_construct(world_pos, ray_dir);
}

// From
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
inline bool intersect_ray_aabb(const Ray *ray, __global AABB *aabb) {
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
inline bool intersect_ray_triangle(const Ray *ray, __global const Triangle *triangle, float *dist) {
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

inline IntersectionInfo Scene_intersect(__global Triangle *triangles, const uint num_triangles,
                                        __global Shape *shapes, const uint num_shapes, Ray *ray) {
    IntersectionInfo intersect_info; // TODO use proper constructor
    intersect_info.m_has_intersected = false;

    // Keep track of closest triangle
    float closest_dist = FLT_MAX;
    Triangle closest_triangle;
    for (unsigned s = 0; s < num_shapes; s++) {
        __global Shape *shape = &(shapes[s]);
        if (intersect_ray_aabb(ray, &(shape->m_aabb))) {
            // for (unsigned i = shape->m_triangle_index_start; i < shape->m_triangle_index_end; i++) {
            //     float dist_to_intersect;
            //     __global Triangle *tri = &(triangles[i]);
            //     bool intersected = intersect_ray_triangle(ray, tri, &dist_to_intersect);
            //     if (intersected) {
            //         // Find closest triangle
            //         if (dist_to_intersect < closest_dist) {
            //             closest_dist = dist_to_intersect;
            //             closest_triangle = *tri;
            //
            //             intersect_info.m_has_intersected = true;
            //             intersect_info.m_pos = ray->m_origin + ray->m_dir * closest_dist;
            //             intersect_info.m_incoming_ray = *ray;
            //             intersect_info.m_angle_between =
            //                 dot(closest_triangle.m_normal, intersect_info.m_incoming_ray.m_dir);
            //             intersect_info.m_normal = closest_triangle.m_normal;
            //             intersect_info.m_material = closest_triangle.m_material;
            //         }
            //     }
            // }
        }
    }

    return intersect_info;
}

__kernel void renderer_trace_camera_ray(__write_only __global float4 *output, const uint width,
                                        const uint max_depth, __global PRNG *prng,
                                        __constant Camera *camera, __global Triangle *triangles,
                                        const uint num_triangles, __global Shape *shapes,
                                        const uint num_shapes) {
    uint x = get_global_id(0);
    uint y = get_global_id(1);
    uint index = y * width + x;

    Ray next_ray = Camera_pixel_to_ray(prng, camera, x, y);
    float3 return_color = (float3)(0.f);
    float3 luminance = (float3)(1.f);
    size_t depth = 0;

    // We'll run until terminated by Russian Roulette
    while (true) {
        // Russian Roulette
        float continuation_probability = 1.f - (1.f / (max_depth - depth));
        // float continuation_probability = (luminance.x + luminance.y + luminance.z) / 3.f;
        if (rand_range(prng, 0.f, 1.f) >= continuation_probability) {
            break;
        }
        depth++;

        IntersectionInfo intersect_info = Scene_intersect(triangles, num_triangles, shapes, num_shapes, &next_ray);
        if (intersect_info.m_has_intersected) {
            // Emitter Material
            if (intersect_info.m_material.m_type == 1) {
                return_color = luminance * intersect_info.m_material.m_color *
                               intersect_info.m_material.m_emittance / continuation_probability;
                break;
            }

            // Diffuse Material
            else if (intersect_info.m_material.m_type == 2) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -sign(intersect_info.m_angle_between);

                // Find new random direction for diffuse reflection

                // We're using importance sampling for this since it converges much faster than
                // uniform sampling
                // See http://blog.hvidtfeldts.net/index.php/2015/01/path-tracing-3d-fractals/ and
                // http://www.rorydriscoll.com/2009/01/07/better-sampling/ and
                // https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
                float3 new_ray_dir =
                    oriented_cosine_weighted_hemisphere_sample(prng, intersect_info.m_normal);
                luminance *= intersect_info.m_material.m_color;

                // For completeness, this is what it looks like with uniform sampling:
                // glm::vec3 new_ray_dir =
                // oriented_uniform_hemisphere_sample(intersect_info.m_normal);
                // float cos_theta = glm::dot(new_ray_dir, intersect_info.m_normal);
                // luminance *= 2.f * intersect_info.m_material.m_color * cos_theta;

                // Make a new ray
                next_ray = ray_construct(intersect_info.m_pos, new_ray_dir);
            }

            // Glass Material
            else if (intersect_info.m_material.m_type == 3) {
                // This code is mostly taken from TomCrypto's Lambda
                float n1, n2;

                if (intersect_info.m_angle_between > 0) {
                    // Ray in inside the object
                    n1 = intersect_info.m_material.m_ior;
                    n2 = 1.0003f;

                    intersect_info.m_normal = -intersect_info.m_normal;
                } else {
                    // Ray is outside the object
                    n1 = 1.0003f;
                    n2 = intersect_info.m_material.m_ior;

                    intersect_info.m_angle_between = -intersect_info.m_angle_between;
                }

                float n = n1 / n2;

                float cos_t = 1.f - pown(n, 2) * (1.f - pown(intersect_info.m_angle_between, 2));

                float3 new_ray_dir;
                // Handle total internal reflection
                if (cos_t < 0.f) {
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                                  (2.f * intersect_info.m_angle_between * intersect_info.m_normal);
                    next_ray = ray_construct(intersect_info.m_pos, new_ray_dir);
                    break;
                }

                cos_t = native_sqrt(cos_t);

                // Fresnel coefficients
                float r1 = n1 * intersect_info.m_angle_between - n2 * cos_t;
                float r2 = n1 * intersect_info.m_angle_between + n2 * cos_t;
                float r3 = n2 * intersect_info.m_angle_between - n1 * cos_t;
                float r4 = n2 * intersect_info.m_angle_between + n1 * cos_t;
                float r = pown(r1 / r2, 2) + pown(r3 / r4, 2) * 0.5f;

                if (rand_range(prng, 0.f, 1.f) < r) {
                    // Reflection
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir -
                                  (2.f * intersect_info.m_angle_between * intersect_info.m_normal);
                    luminance *= intersect_info.m_material.m_color;
                } else {
                    // Refraction
                    new_ray_dir = intersect_info.m_incoming_ray.m_dir * (n1 / n2) +
                                  intersect_info.m_normal *
                                      ((n1 / n2) * intersect_info.m_angle_between - cos_t);
                    luminance *= 1.f;
                }

                // Make a new ray
                next_ray = ray_construct(intersect_info.m_pos, new_ray_dir);
            }

            // Glossy Material
            else if (intersect_info.m_material.m_type == 4) {
                // Find normal in correct direction
                intersect_info.m_normal =
                    intersect_info.m_normal * -sign(intersect_info.m_angle_between);
                intersect_info.m_angle_between =
                    intersect_info.m_angle_between * -sign(intersect_info.m_angle_between);

                float real_roughness = intersect_info.m_material.m_roughness * M_PI_F;

                // Find new direction for reflection
                float3 reflected_dir =
                    intersect_info.m_incoming_ray.m_dir -
                    (2.f * intersect_info.m_angle_between * intersect_info.m_normal);

                // Find new random direction on cone for glossy reflection
                float3 new_ray_dir =
                    oriented_cosine_weighted_cone_sample(prng, reflected_dir, real_roughness);

                luminance *= intersect_info.m_material.m_color;

                // Make a new ray
                next_ray = ray_construct(intersect_info.m_pos, new_ray_dir);
            }
        } else {
            break;
        }
    }

    output[index] = (float4)(return_color.x, return_color.y, return_color.z, 1.f);
}
