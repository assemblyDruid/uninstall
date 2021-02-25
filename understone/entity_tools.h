#ifndef __UE_ENTITY_TOOLS_H___
#define __UE_ENTITY_TOOLS_H___

#include <stdbool.h>

//#include <rt_settings.h> [ cfarvin::REVISIT ]
#include "color_tools.h"
#include "macro_tools.h"
#include "material_tools.h"
#include "maths_tools.h"
#include "type_tools.h"

typedef enum
{
    ET_NONE,
    ET_SPHERE
} EntityType;

typedef struct
{
    r32 radius;
} Sphere;

typedef struct
{
    r32 length;
} Cube;

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4201)
#endif // WIN32
typedef struct
{
    v3       position;
    Material material;

    EntityType type;
    typedef union
    {
        Sphere;
        Cube;
    };
} Entity;
#ifdef _WIN32
#pragma warning(pop)
#endif // WIN32

typedef struct
{
    v3 origin;
    v3 direction;
} Ray;

typedef struct
{
    v3       position;
    v3       normal_vector;
    r32      magnitude;
    bool     does_intersect;
    Material intersection_material;
} RayIntersection;

typedef struct
{
    v3 origin;
    v3 direction;
} Camera;

//
// PROTOTYPES (as needed)
//
static void
IntersectEntity(const Ray* restrict const    ray,
                const Entity* restrict const entity,
                _mut_ RayIntersection* restrict const intersection);

static void
TraceEntityArray(const Ray* restrict const ray,
                 _mut_ RayIntersection* restrict const intersection,
                 _mut_ r32* restrict const global_magnitude_threshold,
                 _mut_ Color32_RGB* restrict const return_color,
                 const Entity* restrict const      entity_arr,
                 const size_t                      num_entitys);

//
#if __UE_AA__reflections
//
static void
ReflectRays(/* const Ray*             restrict const ray, */
            _mut_ RayIntersection* restrict const incident_intersection,
            /* _mut_ r32*             restrict const global_magnitude_threshold,
             */
            _mut_ Color32_RGB* restrict const return_color,
            const Entity* restrict const      entity_arr,
            const size_t                      num_entitys,
            const size_t                      intersected_entity_index)
{
    /* __UE_ASSERT__(ray); */
    __UE_ASSERT__(incident_intersection);
    __UE_ASSERT__(return_color);
    __UE_ASSERT__(entity_arr);
    __UE_ASSERT__(num_entitys >= 1);
    __UE_ASSERT__(intersected_entity_index <= num_entitys);
    /* __UE_ASSERT__(global_magnitude_threshold); */
    __UE_ASSERT__(num_entitys >= 1); // See: TraceEntity( ... );

    if (!incident_intersection->does_intersect)
    {
        return;
    }

    u16             bounces             = 0;
    r64             photon_energy       = 0;
    RayIntersection bounce_intersection = { 0 };
    Ray             bounce_ray          = { 0 };
    Color32_RGB     bounce_color        = { 0 };

    GetEnergyByColorRGB_eV(return_color, &photon_energy);
    photon_energy /= incident_intersection->intersection_material.absorbtion_coefficient;

    while (photon_energy > 0)
    {
        if (bounces > incident_intersection->intersection_material.max_generated_rays)
        {
            break;
        }

        v3Set(&bounce_ray.origin,
              incident_intersection->normal_vector.x,
              incident_intersection->normal_vector.y,
              incident_intersection->normal_vector.z);

        r32 xrand = (r32)XorShift32() + TOLERANCE;
        r32 yrand = (r32)XorShift32() + TOLERANCE;
        r32 zrand = (r32)XorShift32() + TOLERANCE;
        v3SetAndNorm(&bounce_ray.direction,
                     bounce_ray.origin.x + NormalizeToRange((r32)TOLERANCE,
                                                            (r32)(~(u32)0),
                                                            (r32)TOLERANCE,
                                                            (r32)__UE_AA__reflection_noise,
                                                            xrand),
                     bounce_ray.origin.y + NormalizeToRange((r32)TOLERANCE,
                                                            (r32)(~(u32)0),
                                                            (r32)TOLERANCE,
                                                            (r32)__UE_AA__reflection_noise,
                                                            yrand),
                     bounce_ray.origin.z + NormalizeToRange((r32)TOLERANCE,
                                                            (r32)(~(u32)0),
                                                            (r32)TOLERANCE,
                                                            (r32)__UE_AA__reflection_noise,
                                                            zrand));

// Ensure that the reflected ray does not intersect
// the originating entity at a point other than its
// origin.
#if __UE_debug__ == 1
        RayIntersection test_intersection = { 0 };
        IntersectEntity(&bounce_ray, &entity_arr[intersected_entity_index], &test_intersection);
        if (test_intersection.does_intersect)
        {
            __UE_ASSERT__(v3IsEqual(&test_intersection.normal_vector, &bounce_ray.origin));
        }
#endif // __UE_debug__ == 1

        TraceEntityArray(&bounce_ray,
                         &bounce_intersection,
                         &incident_intersection->magnitude,
                         &bounce_color,
                         entity_arr,
                         num_entitys);

        if (bounce_intersection.does_intersect &&
            bounce_intersection.normal_vector.z > incident_intersection->normal_vector.z)
        {
            return_color->channel.R = bounce_color.channel.R;
            return_color->channel.G = bounce_color.channel.G;
            return_color->channel.B = bounce_color.channel.B;
        }

        bounces++;
    }
}
//
#endif // __UE_AA__reflections
//

static void
SetRayDirectionByPixelCoordAA(_mut_ Ray* restrict const ray, const size_t pix_x, const size_t pix_y)
{
    const r32 xOr_contribution_x = NormalizeToRange((r32)TOLERANCE,
                                                    (r32)(~(u32)0),
                                                    (r32)TOLERANCE,
                                                    (r32)__UE_AA__noise,
                                                    (r32)XorShift32() + (r32)TOLERANCE);

    const r32 xOr_contribution_y = NormalizeToRange((r32)TOLERANCE,
                                                    (r32)(~(u32)0),
                                                    (r32)TOLERANCE,
                                                    (r32)__UE_AA__noise,
                                                    (r32)XorShift32() + (r32)TOLERANCE);

    const r32 x_numerator = (r32)pix_x + xOr_contribution_x;
    const r32 y_numerator = (r32)pix_y + xOr_contribution_y;

    ray->direction.x = ((x_numerator / (r32)IMAGE_WIDTH) - 0.5f) * ASPECT_RATIO;
    ray->direction.y = (y_numerator / (r32)IMAGE_HEIGHT) - 0.5f;
    ray->direction.z = -1;
}

static void
SetRayDirectionByPixelCoord(_mut_ Ray* restrict const ray, const size_t pix_x, const size_t pix_y)
{
    ray->direction.x = ((pix_x / (r32)IMAGE_WIDTH) - 0.5f) * ASPECT_RATIO;
    ray->direction.y = (pix_y / (r32)IMAGE_HEIGHT) - 0.5f;
    ray->direction.z = -1;
}

static Entity*
CreateEntities(const size_t entity_count)
{
    return (Entity*)calloc(entity_count, sizeof(Entity));
}

static void
IntersectEntity(const Ray* restrict const    ray,
                const Entity* restrict const entity,
                _mut_ RayIntersection* restrict const intersection)
{
    __UE_ASSERT__(ray && entity && intersection);
    __UE_ASSERT__(v3IsNorm(&ray->direction));

    // Quadratic
    r32 entity_radius_sq = entity->radius * entity->radius;
    v3  ray_to_entity    = { 0 };
    v3Sub(&ray->origin, &entity->position, &ray_to_entity);

    // Ray magnitude must fall within boundaries
    r32 ray_dir_mag = v3Mag(&ray->direction);
    if (!(ray_dir_mag >= MIN_RAY_MAG && ray_dir_mag <= MAX_RAY_MAG))
    {
        intersection->does_intersect = false;
        return;
    }

    r32 a = ray_dir_mag * ray_dir_mag;
    r32 b = 2.0f * (v3Dot(&ray->direction, &ray_to_entity));
    r32 c = v3Dot(&ray_to_entity, &ray_to_entity) - entity_radius_sq;
    /* __UE_ASSERT__(c > 0); */

    r32 discriminant = (b * b) - (4.0f * a * c);
    if (discriminant >= 0.0f)
    {
        intersection->does_intersect = true;
    }
    else
    {
        intersection->does_intersect = false;
    }

    // Build intersection data
    if (intersection->does_intersect)
    {
        // Set intersection magnitude
        r32 magnitude = ((b * -1.0f) - (r32)sqrt(discriminant)) / (2.0f * a);

        // Note: Bounces may be in any direction
        //__UE_ASSERT__(magnitude >= 0);

        intersection->magnitude = magnitude;

        // Set intersection position
        v3Set(&intersection->position,
              ray->origin.x + (magnitude * ray->direction.x),
              ray->origin.y + (magnitude * ray->direction.y),
              ray->origin.z + (magnitude * ray->direction.z));

        // Set intersection normal vector
        v3Sub(&entity->position, &intersection->position, &intersection->normal_vector);

        v3Norm(&intersection->normal_vector);

        // [ cfarvin::REVSIT ] intersection materials
        // Set intersection material
        intersection->intersection_material.max_generated_rays = entity->material
                                                                   .max_generated_rays;
        intersection->intersection_material.material_class = entity->material.material_class;
        intersection->intersection_material.absorbtion_coefficient = entity->material
                                                                       .absorbtion_coefficient;
        intersection->intersection_material.color = entity->material.color;
    }
}

static void
TraceEntity(const Ray* restrict const ray,
            _mut_ RayIntersection* restrict const intersection,
            _mut_ r32* restrict const global_magnitude_threshold,
            _mut_ Color32_RGB* restrict const return_color,
            const Entity* restrict const      entity)
{
    __UE_ASSERT__(ray);
    __UE_ASSERT__(intersection);
    __UE_ASSERT__(return_color);
    __UE_ASSERT__(entity);
    __UE_ASSERT__(global_magnitude_threshold);

    IntersectEntity(ray, entity, intersection);
    if (intersection->does_intersect &&
        (fabs(intersection->magnitude) < fabs(*global_magnitude_threshold)))
    {
        *global_magnitude_threshold = (r32)fabs(intersection->magnitude);
        return_color->value         = entity->material.color.value;
    }
}

static void
TraceEntityArray(const Ray* restrict const ray,
                 _mut_ RayIntersection* restrict const intersection,
                 _mut_ r32* restrict const global_magnitude_threshold,
                 _mut_ Color32_RGB* restrict const return_color,
                 const Entity* restrict const      entity_arr,
                 const size_t                      num_entitys)
{
    __UE_ASSERT__(ray);
    __UE_ASSERT__(intersection);
    __UE_ASSERT__(return_color);
    __UE_ASSERT__(entity_arr);
    __UE_ASSERT__(global_magnitude_threshold);
    __UE_ASSERT__(num_entitys >= 2); // See: TraceEntity( ... );

    RayIntersection closestIntersection = { 0 };
    closestIntersection.magnitude       = MAX_RAY_MAG;
//
#if __UE_AA__reflections
    //
    size_t intersected_entity_index = 0;
//
#endif // __UE_AA__reflections \
    //
    for (size_t entity_index = 0; entity_index < num_entitys; entity_index++)
    {
        IntersectEntity(ray, &entity_arr[entity_index], intersection);
        if (intersection->does_intersect &&
            (intersection->magnitude < closestIntersection.magnitude))
        {
            closestIntersection.does_intersect = true;
            closestIntersection.magnitude      = intersection->magnitude;
            v3Set(&closestIntersection.normal_vector,
                  intersection->normal_vector.x,
                  intersection->normal_vector.y,
                  intersection->normal_vector.z);

            return_color->value = (entity_arr[entity_index]).material.color.value;
//
#if __UE_AA__reflections
            //
            intersected_entity_index = entity_index;
//
#endif // __UE_AA__reflections \
    //
        }
    }

    if (closestIntersection.does_intersect &&
        fabs(closestIntersection.magnitude) < fabs(*global_magnitude_threshold))
    {
        intersection->does_intersect = true;
    }

//
#if __UE_AA__reflections
    //
    ReflectRays(intersection, return_color, entity_arr, num_entitys, intersected_entity_index);
//
#endif // __UE_AA__reflections \
       //
}

static Entity*
CreateRandomEntities(size_t num_entitys)
{
    Entity* entity_arr = CreateEntities(num_entitys);
    for (size_t entity_index = 0; entity_index < num_entitys; entity_index++)
    {
        // Positions
        entity_arr[entity_index].position.x = NormalizeToRange((r32)TOLERANCE,
                                                               (r32)(~(u32)0),
                                                               -1.0f,
                                                               +1.0f,
                                                               (r32)XorShift32());

        entity_arr[entity_index].position.y = NormalizeToRange((r32)TOLERANCE,
                                                               (r32)(~(u32)0),
                                                               -1.0f,
                                                               +1.0f,
                                                               (r32)XorShift32());

        entity_arr[entity_index].position.z = NormalizeToRange((r32)TOLERANCE,
                                                               (r32)(~(u32)0),
                                                               -2.0f,
                                                               -1.0f,
                                                               (r32)XorShift32());

        // Radius
        entity_arr[entity_index].radius = NormalizeToRange((r32)TOLERANCE,
                                                           (r32)(~(u32)0),
                                                           0.15f,
                                                           0.30f,
                                                           (r32)XorShift32());

        // Materials
        entity_arr[entity_index].material.color.channel.R = BindValueTo8BitColorChannel(
          (r32)TOLERANCE,
          (r32)(~(u32)0),
          (r32)XorShift32());
        entity_arr[entity_index].material.color.channel.G = BindValueTo8BitColorChannel(
          (r32)TOLERANCE,
          (r32)(~(u32)0),
          (r32)XorShift32());
        entity_arr[entity_index].material.color.channel.B = BindValueTo8BitColorChannel(
          (r32)TOLERANCE,
          (r32)(~(u32)0),
          (r32)XorShift32());
//
#if __UE_debug__ == 1
        //
        // Log
        printf("  Entity [%zd]:\n", entity_index);
        printf("    Position: (%2.2f, %2.2f, %2.2f)\n",
               entity_arr[entity_index].position.x,
               entity_arr[entity_index].position.y,
               entity_arr[entity_index].position.z);
        printf("    Radius: %2.2f\n", entity_arr[entity_index].radius);
        printf("    Material.color: (%d, %d, %d)\n:",
               entity_arr[entity_index].material.color.channel.R,
               entity_arr[entity_index].material.color.channel.G,
               entity_arr[entity_index].material.color.channel.B);
        fflush(stdout);
//
#endif // __UE_debug__ == 1 \
    //
    }

    return entity_arr;
}

#endif // __UE_ENTITY_TOOLS_H___

// [ cfarvin::TODO ] The following are sphere-only: uCreateEntities(),
// IntersectEntity(), TraceEntity(), TraceEntityArray(), CreateRandomEntities()
