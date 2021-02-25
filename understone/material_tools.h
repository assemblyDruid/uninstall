#ifndef __UE_MATERIAL_TOOLS_H___
#define __UE_MATERIAL_TOOLS_H___

#include <color_tools.h>
#include <macro_tools.h>
#include <rt_settings.h>
#include <type_tools.h>

typedef enum
{
    MATERIAL_CLASS_N__UE_ON__E,
    MATERIAL_CLASS_DIFFUSE,
    MATERIAL_CLASS_METAL
} MaterialClass;

typedef struct
{
    u16           max_generated_rays;
    MaterialClass material_class;
    r32           absorbtion_coefficient;
    Color32_RGB   color;
} Material;

static void
GetDefaultMaterialByClass(_mut_ Material* restrict const material,
                          const MaterialClass            material_class)
{
    __UE_ASSERT__(material);

    switch (material_class)
    {
        case MATERIAL_CLASS_DIFFUSE:
        {
            material->max_generated_rays     = 3;
            material->absorbtion_coefficient = (r32)0.85;
            break;
        }
        case MATERIAL_CLASS_METAL:
        {
            material->max_generated_rays     = 3;
            material->absorbtion_coefficient = (r32)0.05;
            break;
        }

        case MATERIAL_CLASS_N__UE_ON__E:
        {
            material->max_generated_rays     = 0;
            material->absorbtion_coefficient = (r32)0.0f;
            break;
        }
    }
}

static void
BlendColorByMaterial(const Material* restrict const    material,
                     const Color32_RGB* restrict const input_color,
                     _mut_ Color32_RGB* restrict const return_color)
{
    __UE_ASSERT__(material);
    __UE_ASSERT__(material->max_generated_rays > 0);
    __UE_ASSERT__(material->material_class != MATERIAL_CLASS_N__UE_ON__E);
    __UE_ASSERT__(material->absorbtion_coefficient >= 0 && material->absorbtion_coefficient <= 1);
    __UE_ASSERT__(input_color);
    __UE_ASSERT__(return_color);

    // [ cfarvin::TODO ] (return_color->channel.R)

    return;
}

#endif // __UE_MATERIAL_TOOLS_H__
