#ifndef __UE_VULKAN_DEFAULT_CREATE_INFOS_H__
#define __UE_VULKAN_DEFAULT_CREATE_INFOS_H__

#include "vulkan/vulkan.h"

static const VkDeviceCreateInfo default_device_ci = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                                      .pNext = NULL,
                                                      .flags = 0,
                                                      .queueCreateInfoCount    = 0,
                                                      .pQueueCreateInfos       = NULL,
                                                      .enabledLayerCount       = 0,
                                                      .ppEnabledLayerNames     = NULL,
                                                      .enabledExtensionCount   = 0,
                                                      .ppEnabledExtensionNames = NULL,
                                                      .pEnabledFeatures        = NULL };
#if _WIN32
static const VkWin32SurfaceCreateInfoKHR default_win32_surface_ci = {
    .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .pNext     = NULL,
    .flags     = 0,
    .hinstance = NULL,
    .hwnd      = NULL
};
#endif // _WIN32

static const VkDebugUtilsMessengerCreateInfoEXT default_debug_messenger_ci = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = NULL,
    .flags = 0,

    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,

    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,

    .pfnUserCallback = NULL,
    .pUserData       = NULL,
};

//
// [ cfarvin::REMOVE ] NEXT ITER
//

static const r32                     default_device_queue_priority = 1.0f;
static const VkDeviceQueueCreateInfo default_device_queue_ci       = {
    .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .pNext            = NULL,
    .flags            = 0,
    .queueFamilyIndex = 0,
    .queueCount       = 1,
    .pQueuePriorities = &default_device_queue_priority
};

static const VkCommandPoolCreateInfo default_command_pool_ci = {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext            = NULL,
    .flags            = 0,
    .queueFamilyIndex = 0
};

static const VkComponentMapping default_component_mapping = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                              .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                              .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                                              .a = VK_COMPONENT_SWIZZLE_IDENTITY };

static const VkImageSubresourceRange default_sub_resource_range = {
    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
    .baseMipLevel   = 0,
    .levelCount     = 1,
    .baseArrayLayer = 0,
    .layerCount     = 1
};

static const VkImageViewCreateInfo default_image_view_ci = {
    .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .pNext                           = NULL,
    .flags                           = 0,
    .image                           = NULL,
    .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
    .format                          = VK_FORMAT_UNDEFINED,
    .components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
    .components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
    .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
    .subresourceRange.baseMipLevel   = 0,
    .subresourceRange.levelCount     = 1,
    .subresourceRange.baseArrayLayer = 0,
    .subresourceRange.layerCount     = 1
};

// Relates to VkRenderPassCreateInfo.
static const VkAttachmentDescription default_attachment_description = {
    .flags          = 0,
    .format         = VK_FORMAT_UNDEFINED,
    .samples        = VK_SAMPLE_COUNT_1_BIT,
    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
};

// Relates to VkRenderPassCreateInfo.
static const VkAttachmentReference default_attachment_reference = {
    .attachment = 0,
    .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
};

// Relates to VkRenderPassCreateInfo.
static const VkSubpassDependency default_subpass_dependency = {
    .srcSubpass    = VK_SUBPASS_EXTERNAL,                           // First subpass to be run.
    .dstSubpass    = 0,                                             // Index in pSubpasses array.
    .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Source stage to wait on.
    .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Destinatino stage to wait on.
    .srcAccessMask = 0, // Types of operations that wait on srcStageMask.
    .dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, // Types of operations that wait on dstStageMask.
    .dependencyFlags = 0
};

// Relates to VkRenderPassCreateInfo.
static const VkSubpassDescription default_subpass_description = {
    .flags                   = 0,
    .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount    = 0,
    .pInputAttachments       = NULL,
    .colorAttachmentCount    = 0,
    .pColorAttachments       = NULL,
    .pResolveAttachments     = NULL,
    .pDepthStencilAttachment = NULL,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments    = NULL
};

static const VkRenderPassCreateInfo default_render_pass_ci = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext           = NULL,
    .flags           = 0,
    .attachmentCount = 0,
    .pAttachments    = NULL,
    .subpassCount    = 0,
    .pSubpasses      = NULL,
    .dependencyCount = 0,
    .pDependencies   = NULL
};

static const VkGraphicsPipelineCreateInfo default_graphics_pipeline_ci = {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = NULL,
    .flags               = 0,
    .stageCount          = 0,
    .pStages             = NULL,
    .pVertexInputState   = NULL,
    .pInputAssemblyState = NULL,
    .pTessellationState  = NULL,
    .pViewportState      = NULL,
    .pRasterizationState = NULL,
    .pMultisampleState   = NULL,
    .pDepthStencilState  = NULL,
    .pColorBlendState    = NULL,
    .pDynamicState       = NULL,
    .layout              = NULL,
    .renderPass          = NULL,
    .subpass             = 0,
    .basePipelineHandle  = NULL,
    .basePipelineIndex   = -1
};

static const VkFramebufferCreateInfo default_frame_buffer_ci = {
    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .pNext           = NULL,
    .flags           = 0,
    .renderPass      = NULL,
    .attachmentCount = 1,
    .pAttachments    = NULL,
    .width           = 0,
    .height          = 0,
    .layers          = 0
};

static const VkCommandBufferAllocateInfo default_command_buffer_allocate_ci = {
    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext              = NULL,
    .commandPool        = NULL,
    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 0,
};

static const VkCommandBufferBeginInfo default_command_buffer_begin_info = {
    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext            = NULL,
    .flags            = 0,
    .pInheritanceInfo = NULL,
};

static const VkRenderPassBeginInfo default_render_pass_bi = {
    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext           = NULL,
    .renderPass      = NULL,
    .framebuffer     = NULL,
    .renderArea      = { 0 },
    .clearValueCount = 1,
    .pClearValues    = NULL,
};

static const VkShaderModuleCreateInfo default_shader_module_ci = {
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = NULL,
    .flags    = 0,
    .codeSize = 0,
    .pCode    = NULL
};

static const VkPipelineShaderStageCreateInfo default_pipeline_shader_stage_ci = {
    .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .pNext               = NULL,
    .flags               = 0,
    .stage               = 0,
    .module              = 0,
    .pName               = "main",
    .pSpecializationInfo = NULL
};

static const VkSwapchainCreateInfoKHR default_swapchain_ci = {
    .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext                 = NULL,
    .flags                 = 0,
    .surface               = NULL,
    .minImageCount         = 0,
    .imageFormat           = 0,
    .imageColorSpace       = 0,
    .imageExtent           = { 0 },
    .imageArrayLayers      = 0,
    .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // Render directly to images
    .imageSharingMode      = 0,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices   = 0,
    .preTransform          = 0,
    .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // Ignore alpha channel,
    .presentMode           = 0,
    .clipped               = VK_TRUE,
    .oldSwapchain          = NULL
};

static const VkPipelineVertexInputStateCreateInfo default_vertex_input_state_ci = {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext                           = NULL,
    .flags                           = 0,
    .vertexBindingDescriptionCount   = 0,
    .pVertexBindingDescriptions      = NULL,
    .vertexAttributeDescriptionCount = 0,
    .pVertexAttributeDescriptions    = NULL
};

static const VkPipelineInputAssemblyStateCreateInfo default_input_assembly_state_ci = {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext                  = NULL,
    .flags                  = 0,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
};

static const VkViewport default_viewport = { .x        = 0,
                                             .y        = 0,
                                             .width    = 0,
                                             .height   = 0,
                                             .minDepth = 0,
                                             .maxDepth = 1 };

static const VkPipelineViewportStateCreateInfo default_viewport_state_ci = {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext         = NULL,
    .flags         = 0,
    .viewportCount = 1,
    .pViewports    = NULL,
    .scissorCount  = 1,
    .pScissors     = NULL
};

static const VkPipelineRasterizationStateCreateInfo default_rasterization_state_ci = {
    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext                   = NULL,
    .flags                   = 0,
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .cullMode                = 1.0f,
    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp          = 0.0f,
    .depthBiasSlopeFactor    = 0.0f,
    .lineWidth               = 0.0f
};

static const VkPipelineMultisampleStateCreateInfo default_multisample_state_ci = {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext                 = NULL,
    .flags                 = 0,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable   = VK_FALSE,
    .minSampleShading      = 1.0f,
    .pSampleMask           = NULL,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable      = VK_FALSE
};

// [ cfarvin::TODO ] Default Depth Stencil Create Info
/* static const VkPipelineDepthStencilStateCreateInfo default_depth_stencil_ci = { */
/*     VkStructureType                           .sType = , */
/*     const void*                               .pNext = , */
/*     VkPipelineDepthStencilStateCreateFlags    .flags = , */
/*     VkBool32                                  .depthTestEnable = , */
/*     VkBool32                                  .depthWriteEnable = , */
/*     VkCompareOp                               .depthCompareOp = , */
/*     VkBool32                                  .depthBoundsTestEnable = , */
/*     VkBool32                                  .stencilTestEnable = , */
/*     VkStencilOpState                          .front = , */
/*     VkStencilOpState                          .back = , */
/*     float                                     .minDepthBounds = , */
/*     float                                     .maxDepthBounds = */
/* }; */

static const VkPipelineColorBlendAttachmentState default_color_blend_attachment_state = {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
};

static const VkPipelineColorBlendStateCreateInfo default_color_blend_state_ci = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext             = NULL,
    .flags             = 0,
    .logicOpEnable     = VK_FALSE,
    .logicOp           = VK_LOGIC_OP_COPY,
    .attachmentCount   = 1,
    .pAttachments      = &default_color_blend_attachment_state,
    .blendConstants[0] = 0.0f,
    .blendConstants[1] = 0.0f,
    .blendConstants[2] = 0.0f,
    .blendConstants[3] = 0.0f
};

static const VkPipelineDynamicStateCreateInfo default_dynamic_state_ci = {
    .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .pNext             = NULL,
    .flags             = 0,
    .dynamicStateCount = 0,
    .pDynamicStates    = NULL
};

static const VkPipelineLayoutCreateInfo default_pipeline_ci = {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext                  = NULL,
    .flags                  = 0,
    .setLayoutCount         = 0,
    .pSetLayouts            = NULL,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges    = NULL
};

#endif // __UE_VULKAN_DEFAULT_CREATE_INFOS_H__

// [ cfarvin::TODO ] Default Vulkan CI/Struct Updates.
//
// 1. VkAttachmentReference updated to VkAttachmentReference2
// 2. VkSubpassDependency updated to VkSubpassDependency2
// 3. VkSubpassDescription updated to VkSubpassDescription2
