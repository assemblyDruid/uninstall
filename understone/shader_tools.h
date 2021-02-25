#ifndef __UE_SHADER_TOOLS__
#define __UE_SHADER_TOOLS__

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif // _WIN32

#include "maths_tools.h"
#include "type_tools.h"
#include "vulkan_macros.h"

#include <vulkan/vulkan.h>

typedef struct
{
    v2 position;
    v3 color;
} uShaderVertex;

typedef enum
{
    uVK_SHADER_TYPE_NONE = 0,
    uVK_SHADER_TYPE_VERTEX,
    uVK_SHADER_TYPE_FRAGMENT,
    uVK_SHADER_TYPE_COUNT
} uShaderType;

typedef struct
{
    const char*       name;
    const u32*        data;
    const size_t      data_size;
    const uShaderType type;
    VkShaderModule    module; // mutable
} uShader;

/*
 * @brief Creates a VkShaderModule for a uShader. Callers should first see uvkCreateShaders,
 *        which incorporates this functionality.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[out] return_shader The uShader to create a VkShaderModule for. All other
 *             members of the uShader object must be valid and pre-filled.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateShaderModule(const VkDevice logical_device, uShader* const restrict return_shader);

/*
 * @brief Destroys a shader module for a given uShader object. Callers should first see
 *        uvkCreateShaders, which incorporates this functionality.
 *
 * @param[in] logical_dvice A valid VkDevice.
 * @param[in] shader_modules An array of VkShaderModule to destroy.
 * @param[in] shader_module_count The number of elements in the uShader array.
 */
void
uvkDestroyShaderModules(const VkDevice  logical_device,
                        VkShaderModule* shader_modules,
                        const size_t    shader_module_count);

/*
 * @brief A helper function to quickly build VkPipelineShaderStageCreateInfo objects. Callers
 *        should first see uvkCreateShaders, which incorporates this functionality.
 *
 * @param[in] shader A valid and completely filled uShader.
 * @param[out] return_shader_stage_ci The created VkPipelineShaderStageCreateInfo.
 */
void
uvkCreateShaderStageCreateInfo(const uShader* const restrict shader,
                               VkPipelineShaderStageCreateInfo* const restrict
                                 return_shader_stage_ci);

/*
 * @brief Converts a uShaderType enumeration to the Vulkan VkShaderStageFlagBits.
 *
 * @param[in] shader_type A uShaderType.
 *
 * @return The VkShaderStageFlagBits corresponding to uShaderType on success, 0 otherwise.
 */
VkShaderStageFlagBits
uvkShaderTypeToStageBit(uShaderType shader_type);

/*
 * @brief Creates a VkShaderModule and a VkPipelineShaderStageCreateInfo for each uShader.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] shader_count The number of elements in the uShader and stage create info array.
 * @param[in] return_shader_stage_cis An array of VkPipelineShaderStageCreateInfo to be created.
 * @param[in] return_shaders An array of uShader pointers to create VkShaderModules for.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateShaders(const VkDevice                                  logical_device,
                 const u32                                       shader_count,
                 VkPipelineShaderStageCreateInfo* const restrict return_shader_stage_cis,
                 uShader** const restrict                        return_shaders);

#endif // __UE_SHADER_TOOLS__
