#include "shader_tools.h"

#include <stdbool.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif // _WIN32

#include "debug_tools.h"
#include "macro_tools.h"
#include "maths_tools.h"
#include "type_tools.h"
#include "vulkan_default_create_infos.h"
#include "vulkan_macros.h"

static bool
uvkValidateShaderType(uShaderType shader_type)
{
    if (shader_type == uVK_SHADER_TYPE_VERTEX || shader_type == uVK_SHADER_TYPE_FRAGMENT)
    {
        return true;
    }

    return false;
}

VkShaderStageFlagBits
uShaderTypeToStageBit(uShaderType shader_type)
{
    // Consider 0 stage bit invalid.
    VkShaderStageFlagBits retVal = (VkShaderStageFlagBits)0;
    switch (shader_type)
    {
        case uVK_SHADER_TYPE_VERTEX:
        {
            retVal = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        }
        case uVK_SHADER_TYPE_FRAGMENT:
        {
            retVal = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        }
        default:
        {
            retVal = (VkShaderStageFlagBits)0;
            break;
        }
    };

    return retVal;
}

void
uvkCreateShaderStageCreateInfo(const uShader* const restrict shader,
                               VkPipelineShaderStageCreateInfo* const restrict
                                 return_shader_stage_ci)
{
    uAssertMsg_v(NULL != shader, "[ shaders ] uShader ptr must be non null.\n");
    uAssertMsg_v(NULL != return_shader_stage_ci,
                 "[ shaders ] VkPipelineShaderStageCreateInfo ptr must be non null.\n");

    *return_shader_stage_ci        = default_pipeline_shader_stage_ci;
    return_shader_stage_ci->stage  = uShaderTypeToStageBit(shader->type);
    return_shader_stage_ci->module = shader->module;
}

bool
uvkCreateShaderModule(const VkDevice logical_device, uShader* const restrict return_shader)
{
    uAssertMsg_v(NULL != logical_device, "[ shaders ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != return_shader, "[ shaders ] uShader ptr must be non null.\n");
    uAssertMsg_v(0 == return_shader->module,
                 "[ shaders ] VkShaderModule must be zero; will be overwritten\n.");
    uAssertMsg_v(NULL != return_shader->data, "[ shaders ] Shader data ptr must be non null\n.");
    uAssertMsg_v(0 != return_shader->data_size,
                 "[ shaders ] Shader byte count must be non zero\n.");
    uAssertMsg_v(false != uvkValidateShaderType(return_shader->type),
                 "[ shaders ] Shader type must be valid.\n");

    VkShaderModuleCreateInfo shader_module_ci = default_shader_module_ci;
    shader_module_ci.codeSize                 = return_shader->data_size;
    shader_module_ci.pCode                    = return_shader->data;

    VkResult result = vkCreateShaderModule(logical_device,
                                           &shader_module_ci,
                                           NULL,
                                           &(return_shader->module));

    if (VK_SUCCESS != result)
    {
        uError("[ shaders ] Unable to create shader module.\n");
        return false;
    }

    return true;
}

void
uvkDestroyShaderModules(const VkDevice                 logical_device,
                        VkShaderModule* const restrict shader_modules,
                        const size_t                   shader_module_count)
{
    uAssertMsg_v(NULL != logical_device, "[ shaders ] VkDevice must be non null.\n");
    uAssertMsg_v(shader_modules, "[ shaders ] VkShaderModule ptr must be non null.\n");
    uAssertMsg_v(shader_module_count, "[ shaders ] Shader module count must be non zero.\n");

    for (size_t module_idx = 0; module_idx < shader_module_count; module_idx++)
    {
        VkShaderModule module = shader_modules[module_idx];
        vkDestroyShaderModule(logical_device, module, NULL);
        module = 0;
    }
}

bool
uvkCreateShaders(const VkDevice                                  logical_device,
                 const u32                                       shader_count,
                 VkPipelineShaderStageCreateInfo* const restrict return_shader_stage_cis,
                 uShader** const restrict                        return_shaders)
{
    uAssertMsg_v(NULL != logical_device, "[ shaders ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != return_shaders, "[ shaders ] uShader ptr must be non null.\n");
    uAssertMsg_v(0 != shader_count, "[ shaders ] Shader count must be non zero.\n");
    uAssertMsg_v(NULL != return_shader_stage_cis,
                 "[ shaders ] VkPipelineShaderStageCreateInfo ptr must be non null.\n");

    for (size_t shader_idx = 0; shader_idx < shader_count; shader_idx++)
    {
        // Create shader modules.
        if (false == uvkCreateShaderModule(logical_device, return_shaders[shader_idx]))
        {
            uError("[ shader ] Unable to create shader; module creation failed.\n");
            return false;
        }

        // Create stage info after successful module creation.
        uvkCreateShaderStageCreateInfo(return_shaders[shader_idx],
                                       &(return_shader_stage_cis[shader_idx]));
    }

    return true;
}
