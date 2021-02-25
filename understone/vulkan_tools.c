// [cfarvin::TODO] Debug implement
//
// Something like this would be REALLY helpful in debugging in the
// appropriate function header contexts:
//    uDebugStatement(static char times_called = 1);
//    uDebugStatement(const u8 expected_times_called = <X>);
//    uDebugStatement(times_called++);
//    uAssertMsg_v(times_called < expected_times_called) );

// Note: Skip window manager (or build a cusom one) with VK_KHR_display
// && VK_KHR_display_swapchain to render fullscreen.

// [ cfarvin::TODO ] Correctness uAssertMsg_v
//
// All vk non dispatchable handles can be checked for null as in the following:
// uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");

// [ cfarvin::TODO ] Future correctness
//
// Right now, we're passing all of the VK*CreateInfo(s) by const value to make sure
// that everything is working. In practice, this means that the user will be filling
// them out on the stack before a function call, and then we're doing a full copy to
// pass to the appropriate function. After everything is working, we should change the
// const Vk*CreateInfo paramters to be pointers to avoid this copy.

// [ cfarvin::TODO ] Future memfree correctness
//
// Go through to find all calloc/mallocs in Vulkan setup functions, and make sure that
// on conditions where uError() is being called for failures, that any pointers which
// have been allocated in the scope of the function are freed.

#include <stdbool.h>
#include <stdio.h>

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif // _WIN32

#include "vulkan/vulkan.h"
#include "vulkan_tools.h"

#if _WIN32
#include <understone/win_platform.h>
#endif // _WIN32

#include "engine_info.h"
#include "memory_tools.h"
#include "shader_tools.h"
#include "vulkan_default_create_infos.h"
#include "vulkan_macros.h"
#include "window_tools.h"

extern const u8 kNumVulkanCommandBuffers;

VKAPI_ATTR VkBool32 VKAPI_CALL
uvkDebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity_bits,
                  VkDebugUtilsMessageTypeFlagsEXT             message_type_bits,
                  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                  void*                                       user_data)
{
    printf("%s\n", callback_data->pMessage);
    fflush(stdout);

    if (message_severity_bits >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        char fgets_buff[8] = { 0 };
        puts("[ vk validation ]");
        fflush(stdout);

        // Hang to notify.
        fgets(fgets_buff, 8, stdin);
    }

    return false;
}

// [ cfarvin::TODO ] Future correctness.
//
// Needs from calling code:
// 6. Subpass dependecy and count (1).
//
// This function presently only makes use of a single subpass. For mobile GPUs, we need
// to expand it or create another function to weild multiple subpasses.
bool
uvkCreateRenderPass(const VkDevice                                logical_device,
                    const VkRenderPassCreateFlags                 render_pass_create_flags,
                    const VkSubpassDescriptionFlags               subpass_description_flags,
                    VkPipelineBindPoint                           pipeline_bind_point,
                    const VkAttachmentDescription* const restrict attachment_descriptions,
                    const u32                                     attachment_description_count,
                    const VkAttachmentReference* const restrict   input_attachment_references,
                    const u32                                     input_attachment_reference_count,
                    const VkAttachmentReference* const restrict   color_attachment_references,
                    const VkAttachmentReference* const restrict   resolve_attachment_references,
                    const u32                                     color_attachment_reference_count,
                    const u32* const restrict                     preserve_attachments,
                    const u32                                     preserve_attachment_count,
                    const VkAttachmentReference* const restrict depth_stencil_attachment_reference,
                    const VkSubpassDependency* const restrict   subpass_dependencies,
                    const u32                                   subpass_dependency_count,
                    VkRenderPass* const restrict                return_render_pass)
{
    // [ cfarvin::TODO ] uAssertMsg_v
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(return_render_pass, "[ vulkan ] Return VkRenderPass ptr must be non null.\n");

    const VkSubpassDescription subpass_description = {
        .flags                   = subpass_description_flags,
        .pipelineBindPoint       = pipeline_bind_point,
        .inputAttachmentCount    = input_attachment_reference_count,
        .pInputAttachments       = input_attachment_references,
        .colorAttachmentCount    = color_attachment_reference_count,
        .pColorAttachments       = color_attachment_references,
        .pResolveAttachments     = resolve_attachment_references,
        .pDepthStencilAttachment = depth_stencil_attachment_reference,
        .preserveAttachmentCount = preserve_attachment_count,
        .pPreserveAttachments    = preserve_attachments
    };

    // Render pass
    VkRenderPassCreateInfo render_pass_ci = default_render_pass_ci;
    render_pass_ci.flags                  = render_pass_create_flags;
    render_pass_ci.attachmentCount        = attachment_description_count;
    render_pass_ci.pAttachments           = attachment_descriptions;
    render_pass_ci.subpassCount           = 1;
    render_pass_ci.pSubpasses             = &subpass_description;
    render_pass_ci.dependencyCount        = subpass_dependency_count;
    render_pass_ci.pDependencies          = subpass_dependencies;

    VkResult result = vkCreateRenderPass(logical_device, &render_pass_ci, NULL, return_render_pass);

    if (VK_SUCCESS != result)
    {
        uError("[ vulkan ] Unable to create render pass.\n");
        return false;
    }

    return true;
}

void
uvkDestroyRenderPass(const VkDevice logical_device, VkRenderPass render_pass)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != render_pass, "[ vulkan ] VkRenderPass  must be non null.\n");

    vkDestroyRenderPass(logical_device, render_pass, NULL);
}

bool
uvkCreateGraphicsPipeline(const VkDevice                                        logical_device,
                          const u32                                             shader_stage_count,
                          const VkPipelineShaderStageCreateInfo* const restrict shader_stage_ci,
                          const VkPipelineVertexInputStateCreateInfo   vertex_input_state_ci,
                          const VkPipelineInputAssemblyStateCreateInfo input_assembly_state_ci,
                          const VkPipelineViewportStateCreateInfo      viewport_state_ci,
                          const VkPipelineRasterizationStateCreateInfo rasterization_state_ci,
                          const VkPipelineMultisampleStateCreateInfo   multisample_state_ci,
                          const VkPipelineDepthStencilStateCreateInfo  depth_stencil_state_ci,
                          const VkPipelineColorBlendStateCreateInfo    color_blend_state_ci,
                          const VkPipelineDynamicStateCreateInfo       dynamic_state_ci,
                          const VkPipelineLayoutCreateInfo             pipeline_layout_ci,
                          const VkRenderPass                           render_pass,
                          const u32                                    subpass_index,
                          VkPipelineLayout* const restrict             return_pipeline_layout,
                          VkPipeline* const restrict                   return_graphics_pipeline)
{
    // [ cfarvin::TODO ] uAssertMsg_v()
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != return_pipeline_layout,
                 "[ vulkan ] Return VkPipelineLayout ptr must be non null.\n");
    uAssertMsg_v(NULL != return_graphics_pipeline,
                 "[ vulkan ] Return VkPipeline ptr must be non null.\n");

    // Create the pipeline layout.
    VkResult success = vkCreatePipelineLayout(logical_device,
                                              &pipeline_layout_ci,
                                              NULL,
                                              return_pipeline_layout);

    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to create a pipeline layout.\n");
        return false;
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_ci = default_graphics_pipeline_ci;
    graphics_pipeline_ci.stageCount                   = shader_stage_count;
    graphics_pipeline_ci.pStages                      = shader_stage_ci;
    graphics_pipeline_ci.pVertexInputState            = &vertex_input_state_ci;
    graphics_pipeline_ci.pInputAssemblyState          = &input_assembly_state_ci;
    graphics_pipeline_ci.pViewportState               = &viewport_state_ci;
    graphics_pipeline_ci.pRasterizationState          = &rasterization_state_ci;
    graphics_pipeline_ci.pMultisampleState            = &multisample_state_ci;
    graphics_pipeline_ci.pDepthStencilState           = &depth_stencil_state_ci;
    graphics_pipeline_ci.pColorBlendState             = &color_blend_state_ci;
    graphics_pipeline_ci.pDynamicState                = &dynamic_state_ci;
    graphics_pipeline_ci.layout                       = *return_pipeline_layout;
    graphics_pipeline_ci.renderPass                   = render_pass;
    graphics_pipeline_ci.subpass                      = subpass_index;

    success = vkCreateGraphicsPipelines(logical_device,
                                        VK_NULL_HANDLE,
                                        1,
                                        &graphics_pipeline_ci,
                                        NULL,
                                        return_graphics_pipeline);
    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to create a graphics pipeline.\n");
        return false;
    }

    return true;
}

void
uvkDestroyPipelineLayout(const VkDevice logical_device, VkPipelineLayout pipeline_layout)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != pipeline_layout, "[ vulkan ] VkPipelineLayout must be non null.\n");

    vkDestroyPipelineLayout(logical_device, pipeline_layout, NULL);
}

void
uvkDestroyPipeline(const VkDevice logical_device, VkPipeline pipeline)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != pipeline, "[ vulkan ] VkPipeline must be non null.\n");

    vkDestroyPipeline(logical_device, pipeline, NULL);
}

// void
// uCreateVulkanGraphicsPipeline(const size_t num_shader_stage_cis,
//                               const VkPipelineShaderStageCreateInfo* const restrict
//                                 shader_stage_cis)
// {
//     // [ cfarvin::TODO ] uAssertMsg_v()

//     // Vertex input
//     VkPipelineVertexInputStateCreateInfo vertex_input_ci = { 0 };
//     vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//     vertex_input_ci.vertexBindingDescriptionCount   = 0;
//     vertex_input_ci.pVertexBindingDescriptions      = NULL;
//     vertex_input_ci.vertexAttributeDescriptionCount = 0;
//     vertex_input_ci.pVertexAttributeDescriptions    = NULL;

//     // Input assembly
//     VkPipelineInputAssemblyStateCreateInfo input_assembly_ci = { 0 };
//     input_assembly_ci.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//     input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//     input_assembly_ci.primitiveRestartEnable = VK_FALSE;

//     // Viewport
//     VkViewport frame_buffer_viewport = { 0 };
//     frame_buffer_viewport.x          = 0.0f;
//     frame_buffer_viewport.y          = 0.0f;
//     frame_buffer_viewport.width      = (r32)surface_info->surface_capabilities.currentExtent.width;
//     frame_buffer_viewport.height     = (r32)surface_info->surface_capabilities.currentExtent.height;
//     frame_buffer_viewport.minDepth   = 0.0f;
//     frame_buffer_viewport.maxDepth   = 1.0f;

//     // Scissor
//     VkRect2D frame_buffer_scissor = { 0 };
//     frame_buffer_scissor.offset.x = 0;
//     frame_buffer_scissor.offset.y = 0;
//     frame_buffer_scissor.extent   = surface_info->surface_capabilities.currentExtent;

//     // Viewport state
//     VkPipelineViewportStateCreateInfo frame_buffer_viewport_ci = { 0 };
//     frame_buffer_viewport_ci.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//     frame_buffer_viewport_ci.viewportCount = 1;
//     frame_buffer_viewport_ci.pViewports    = &frame_buffer_viewport;
//     frame_buffer_viewport_ci.scissorCount  = 1;
//     frame_buffer_viewport_ci.pScissors     = &frame_buffer_scissor;

//     // Rasterizer
//     VkPipelineRasterizationStateCreateInfo raster_ci = { 0 };
//     raster_ci.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//     raster_ci.depthClampEnable        = VK_FALSE;
//     raster_ci.rasterizerDiscardEnable = VK_FALSE;
//     raster_ci.polygonMode             = VK_POLYGON_MODE_FILL;
//     raster_ci.lineWidth               = 1.0f;
//     raster_ci.cullMode                = VK_CULL_MODE_BACK_BIT;
//     raster_ci.frontFace               = VK_FRONT_FACE_CLOCKWISE;
//     raster_ci.depthBiasEnable         = VK_FALSE;
//     raster_ci.depthBiasConstantFactor = 0.0f;
//     raster_ci.depthBiasClamp          = 0.0f;
//     raster_ci.depthBiasSlopeFactor    = 0.0f;

//     // Multisampling
//     VkPipelineMultisampleStateCreateInfo multi_sample_ci = { 0 };
//     multi_sample_ci.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//     multi_sample_ci.sampleShadingEnable  = VK_FALSE;
//     multi_sample_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//     multi_sample_ci.minSampleShading     = 1.0f;
//     multi_sample_ci.pSampleMask          = NULL;
//     multi_sample_ci.alphaToCoverageEnable = VK_FALSE;
//     multi_sample_ci.alphaToOneEnable      = VK_FALSE;

//     // Depth stencil [ currently off ]
//     VkPipelineDepthStencilStateCreateInfo* depth_stencil_ci = NULL;

//     // Color blending
//     // Note: VkPipelineColorBlendStateCreateInfo builds global state.
//     //       VkPipelineColorBlendAttachmentState sets local state per attached
//     //       frame buffer.
//     VkPipelineColorBlendAttachmentState color_blend_attachment = { 0 };
//     color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
//                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//     color_blend_attachment.blendEnable         = VK_FALSE;
//     color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
//     color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//     color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
//     color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//     color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//     color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;

//     VkPipelineColorBlendStateCreateInfo color_blend_ci = { 0 };
//     color_blend_ci.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//     color_blend_ci.logicOpEnable     = VK_FALSE;
//     color_blend_ci.logicOp           = VK_LOGIC_OP_COPY;
//     color_blend_ci.attachmentCount   = 1;
//     color_blend_ci.pAttachments      = &color_blend_attachment;
//     color_blend_ci.blendConstants[0] = 0.0f;
//     color_blend_ci.blendConstants[1] = 0.0f;
//     color_blend_ci.blendConstants[2] = 0.0f;
//     color_blend_ci.blendConstants[3] = 0.0f;

//     // Dynamic state
//     // Note: Causes previous configuration to be ignored, and information to be
//     // supplied at drawing time.
//     VkDynamicState dynamic_states[2] = { 0 };
//     dynamic_states[0]                = VK_DYNAMIC_STATE_VIEWPORT;
//     dynamic_states[1]                = VK_DYNAMIC_STATE_LINE_WIDTH;

//     VkPipelineDynamicStateCreateInfo dynamic_state_ci = { 0 };
//     dynamic_state_ci.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//     dynamic_state_ci.dynamicStateCount = 2;
//     dynamic_state_ci.pDynamicStates    = dynamic_states;

//     // Pipeline layout
//     VkPipelineLayoutCreateInfo pipeline_layout_ci = { 0 };
//     pipeline_layout_ci.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//     pipeline_layout_ci.setLayoutCount             = 0;
//     pipeline_layout_ci.pSetLayouts                = NULL;
//     pipeline_layout_ci.pushConstantRangeCount     = 0;
//     pipeline_layout_ci.pPushConstantRanges        = NULL;

//     VkResult result = vkCreatePipelineLayout(v_info->logical_device,
//                                              &pipeline_layout_ci,
//                                              NULL,
//                                              (VkPipelineLayout*)&(render_info->pipeline_layout));
//     if (result != VK_SUCCESS)
//     {
//         uFatal("[ vulkan ] Unable to create pipeline layout.\n");
//     }

//     // [ cfarvin::TODO ] Graphics Pipeline
//     //
//     // In the future, it will be useful to use the basePipelineHandle and basePipelineIndex members to derive
//     // a new graphics pipeline from an existing one.
//     // Create graphics pipeline
//     VkGraphicsPipelineCreateInfo graphics_pipeline_ci = { 0 };
//     graphics_pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

//     graphics_pipeline_ci.stageCount          = num_shader_stage_cis;
//     graphics_pipeline_ci.pStages             = shader_stage_cis;
//     graphics_pipeline_ci.pVertexInputState   = &vertex_input_ci;
//     graphics_pipeline_ci.pInputAssemblyState = &input_assembly_ci;
//     graphics_pipeline_ci.pViewportState      = &frame_buffer_viewport_ci;
//     graphics_pipeline_ci.pRasterizationState = &raster_ci;
//     graphics_pipeline_ci.pMultisampleState   = &multi_sample_ci;
//     graphics_pipeline_ci.pDepthStencilState  = depth_stencil_ci; // Currently off
//     graphics_pipeline_ci.pColorBlendState    = &color_blend_ci;
//     graphics_pipeline_ci.pDynamicState       = NULL; // Filled out, not using.
//     graphics_pipeline_ci.layout              = render_info->pipeline_layout;
//     graphics_pipeline_ci.renderPass          = render_info->render_pass;
//     graphics_pipeline_ci.subpass             = 0; // Read: "Subpass index"
//     graphics_pipeline_ci.basePipelineHandle  = NULL;
//     graphics_pipeline_ci.basePipelineIndex   = -1;

//     result = vkCreateGraphicsPipelines(v_info->logical_device,
//                                        VK_NULL_HANDLE,
//                                        1,
//                                        &graphics_pipeline_ci,
//                                        NULL,
//                                        (VkPipeline*)&(render_info->graphics_pipeline));

//     if (result != VK_SUCCESS)
//     {
//         uFatal("[ vulkan ] Unable to create graphics pipeline.\n");
//     }
// }

bool
uvkCreateFrameBuffers(const VkDevice                    logical_device,
                      const u32                         image_count,
                      const VkImageView* const restrict image_view_attachments,
                      const VkFramebufferCreateInfo     frame_buffer_ci,
                      VkFramebuffer* restrict           return_frame_buffers)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(0 != image_count, "[ vulkan ] Image count must be non zero.\n");
    uAssertMsg_v(NULL != image_view_attachments, "[ vulkan ] VkImageView ptr must be non null.\n");
    uAssertMsg_v(NULL != return_frame_buffers, "[ vulkan ] VkFramebuffer ptr must be non zero.\n");
    uAssertMsg_v(1 == frame_buffer_ci.attachmentCount,
                 "[ vulkan ] Attachment count must be 1. See function desc in header.\n");

    // Allocate a frame buffer for each image view.
    return_frame_buffers = (VkFramebuffer*)calloc(image_count, sizeof(VkFramebuffer));
    if (NULL == return_frame_buffers)
    {
        uError("[ vulkan ] Unable to allocate frame buffer memory.\n");
        return false;
    }

    // Create the frame buffers.
    VkResult                success             = VK_SUCCESS;
    VkFramebufferCreateInfo tmp_frame_buffer_ci = frame_buffer_ci;
    for (size_t buffer_idx = 0; buffer_idx < image_count; buffer_idx++)
    {
        tmp_frame_buffer_ci.pAttachments = &(image_view_attachments[buffer_idx]);
        success                          = vkCreateFramebuffer(logical_device,
                                      &tmp_frame_buffer_ci,
                                      NULL,
                                      &(return_frame_buffers[buffer_idx]));
        if (VK_SUCCESS != success)
        {
            uError("[ vulkan ] Unable to create frame buffer.\n");
            free(return_frame_buffers);
            return false;
        }
    }

    return true;
}

void
uvkDestroyFrameBuffers(const VkDevice logical_device,
                       VkFramebuffer* frame_buffers,
                       const u32      frame_buffer_count)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != frame_buffers, "[ vulkan ] VkFrameBuffer pointer must be non null.\n");
    uAssertMsg_v(0 != frame_buffer_count, "[ vulkan ] Frame buffer count must be non zero.\n");

    for (size_t frame_buffer_iter = 0; frame_buffer_iter < frame_buffer_count; frame_buffer_iter++)
    {
        VkFramebuffer frame_buffer = frame_buffers[frame_buffer_iter];
        if (NULL != frame_buffer)
        {
            vkDestroyFramebuffer(logical_device, frame_buffer, NULL);
        }
        else
        {
            uWarning("[ vulkan ] Unable to free NULL VkFrameBuffer at index %zd.\n",
                     frame_buffer_iter);
        }
    }

    free(frame_buffers);
    frame_buffers = NULL;
}

// void
// uCreateVulkanFrameBuffers()
// {
//     // [ cfarvin::TODO ] Correctness
//     //
//     // We're not doing things this way any more, but the check for null frame
//     // buffers is still valid when where rebuilding the swapchain. Need to
//     // implement this in the right place.
//     /* uAssertMsg_v(is_rebuilding_swapchain || !image_group->frame_buffers, "[
//      * vulkan ] VkFrameBuffer ptr must be null; will be overwritten.\n"); */

//     // Create a frame buffer for each image view
//     *(VkFramebuffer**)&(image_group->frame_buffers) = (VkFramebuffer*)calloc(
//       image_group->num_images,
//       sizeof(VkFramebuffer));
//     if (!(image_group->frame_buffers))
//     {
//         uFatal("[ vulkan ] Unable to allocate frame buffer array.\n");
//     }

//     VkFramebufferCreateInfo frame_buffer_ci = { 0 };
//     frame_buffer_ci.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//     frame_buffer_ci.renderPass              = render_info->render_pass;
//     frame_buffer_ci.attachmentCount         = 1;
//     frame_buffer_ci.width  = surface_info->surface_capabilities.currentExtent.width;
//     frame_buffer_ci.height = surface_info->surface_capabilities.currentExtent.height;
//     frame_buffer_ci.layers = 1; // Non stereoscopic

//     for (u32 image_idx = 0; image_idx < image_group->num_images; image_idx++)
//     {
//         frame_buffer_ci.pAttachments = &(image_group->image_views[image_idx]);
//         VkResult result              = vkCreateFramebuffer(v_info->logical_device,
//                                               &frame_buffer_ci,
//                                               NULL,
//                                               &(image_group->frame_buffers[image_idx]));

//         if (result != VK_SUCCESS)
//         {
//             uFatal("[ vulkan ] Unable to create frame buffer.\n");
//         }
//     }
// }

bool
uvkCreateCommandBuffers(const VkDevice                    logical_device,
                        const VkCommandBufferAllocateInfo command_buffer_ai,
                        VkCommandBuffer** const restrict  return_command_buffers)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(0 != command_buffer_ai.commandBufferCount,
                 "[ vulkan ] Command buffer count must be non zero.\n");
    uAssertMsg_v(NULL != return_command_buffers,
                 "[ vulkan ] Return VkCommandBuffer ptr ptr must be non null.\n");
    uAssertMsg_v(NULL == *return_command_buffers,
                 "[ vulkan ] Return VkCommandBuffer ptr must be null; will be overwritten.\n");

    // Allocate command buffer(s) locally.
    *return_command_buffers = (VkCommandBuffer*)calloc(command_buffer_ai.commandBufferCount,
                                                       sizeof(VkCommandBuffer));
    if (NULL == *return_command_buffers)
    {
        uError("[ vulkan ] Unable to allocate command buffer memory.\n");
        return false;
    }

    // Allocate command buffer(s) via Vulkan.
    VkResult success = vkAllocateCommandBuffers(logical_device,
                                                &command_buffer_ai,
                                                *return_command_buffers);
    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to allocate command buffer (vk).\n");
        return false;
    }

    return true;
}

bool
uvkDestroyCommandBuffers(const VkDevice      logical_device,
                         const VkCommandPool command_pool,
                         VkCommandBuffer*    command_buffers,
                         const u32           command_buffer_count)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != command_buffers, "[ vulkan ] VkCommandBuffers ptr must be non null.\n");
    uAssertMsg_v(0 != command_buffer_count, "[ vulkan ] Command buffer count must be non zero.\n");

    if ((NULL != command_buffers) && (0 != command_buffer_count))
    {
        vkFreeCommandBuffers(logical_device, command_pool, command_buffer_count, command_buffers);
        command_buffers = NULL;
    }
    else
    {
        uWarning("[ vulkan ] Unable to destroy command buffers.\n");
        return false;
    }

    return true;
}

bool
uvkBeginCommandBuffer(VkCommandBuffer                command_buffer,
                      const VkCommandBufferBeginInfo command_buffer_bi)
{
    VkResult success = vkBeginCommandBuffer(command_buffer, &command_buffer_bi);

    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to begin command buffer.\n");
        return false;
    }

    return true;
}

bool
uvkEndCommandBuffer(VkCommandBuffer command_buffer)
{
    VkResult success = vkEndCommandBuffer(command_buffer);

    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to end command buffer.\n");
        return false;
    }

    return true;
}

// [ cfarvin::REMOVE ] Correctness
void
uvkTriangleRecordCommandBuffers_REMOVE(const VkPipeline            graphics_pipeline,
                                       const VkRenderPassBeginInfo render_pass_bi,
                                       const VkSubpassContents     subpass_contents,
                                       const VkPipelineBindPoint   pipeline_bind_point,
                                       const VkPipeline            pipeline,
                                       VkCommandBuffer             command_buffer)
{
    // Begin render pass.
    vkCmdBeginRenderPass(command_buffer, &render_pass_bi, subpass_contents);

    // Bind pipeline.
    vkCmdBindPipeline(command_buffer, pipeline_bind_point, pipeline);

    // Draw.
    vkCmdDraw(command_buffer,
              3,  // Vertex count.
              1,  // Instance count.
              0,  // First vertext index.
              0); // First instance index.

    // End render pass.
    vkCmdEndRenderPass(command_buffer);
}

// void
// uCreateVulkanCommandBuffers()
// {
//     // Create command buffers
//     if (image_group->num_images > kNumVulkanCommandBuffers)
//     {
//         uFatal("[ api ] More swapchain images than command buffers: (%d, %d).\n",
//                image_group->num_images,
//                kNumVulkanCommandBuffers);
//     }

//     *(VkCommandBuffer**)&(command_info->command_buffers) = (VkCommandBuffer*)calloc(
//       image_group->num_images,
//       sizeof(VkCommandBuffer));

//     if (!command_info->command_buffers)
//     {
//         uFatal("[ vulkan ] Unable to allocate command buffers.\n");
//     }

//     // Note: cmd buffer count set in struct for outside use; image number used
//     // internally.
//     *(u32*)&(command_info->num_command_buffers) = image_group->num_images;

//     VkCommandBufferAllocateInfo cb_alloc_ci = { 0 };
//     cb_alloc_ci.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//     cb_alloc_ci.commandPool                 = command_info->command_pool;
//     cb_alloc_ci.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//     cb_alloc_ci.commandBufferCount          = image_group->num_images;

//     VkResult result = vkAllocateCommandBuffers(v_info->logical_device,
//                                                &cb_alloc_ci,
//                                                (VkCommandBuffer*)command_info->command_buffers);

//     if (result != VK_SUCCESS)
//     {
//         uFatal("[ vulkan ] Unable to create command buffers.\n");
//     }

//     for (u32 image_idx = 0; image_idx < image_group->num_images; image_idx++)
//     {
//         // Begin cb recording
//         VkCommandBufferBeginInfo cb_begin_info = { 0 };
//         cb_begin_info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         cb_begin_info.flags                    = 0;
//         cb_begin_info.pInheritanceInfo         = NULL;

//         result = vkBeginCommandBuffer(command_info->command_buffers[image_idx], &cb_begin_info);

//         if (result != VK_SUCCESS)
//         {
//             uFatal("[ vulkan ] Unable to begin command buffer at index: %d.\n", image_idx);
//         }

//         // Begin render passs
//         VkClearValue clear_value     = { 0 };
//         clear_value.color.float32[3] = 1.0f;

//         VkRenderPassBeginInfo rp_begin_info = { 0 };
//         rp_begin_info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//         rp_begin_info.renderPass            = render_info->render_pass;
//         rp_begin_info.framebuffer           = image_group->frame_buffers[image_idx];
//         rp_begin_info.renderArea.offset.x   = 0;
//         rp_begin_info.renderArea.offset.y   = 0;
//         rp_begin_info.renderArea.extent     = surface_info->surface_capabilities.currentExtent;
//         rp_begin_info.clearValueCount       = 1;
//         rp_begin_info.pClearValues          = &clear_value;

//         vkCmdBeginRenderPass(command_info->command_buffers[image_idx],
//                              &rp_begin_info,
//                              VK_SUBPASS_CONTENTS_INLINE);

//         vkCmdBindPipeline(command_info->command_buffers[image_idx],
//                           VK_PIPELINE_BIND_POINT_GRAPHICS,
//                           render_info->graphics_pipeline);

//         vkCmdDraw(command_info->command_buffers[image_idx],
//                   3,  // vertex count
//                   1,  // instance count
//                   0,  // first vertex
//                   0); // first instance

//         vkCmdEndRenderPass(command_info->command_buffers[image_idx]);

//         result = vkEndCommandBuffer(command_info->command_buffers[image_idx]);
//         if (result != VK_SUCCESS)
//         {
//             uFatal("[ vulkan ] Unable to end command buffer.\n");
//         }
//     }
// }

// [ cfarvin::TODO ] Correctness
//
// This function will need to take more "return" parameters - like the new
// images and framebufers and present modes which will be replaced. Otherwise,
// the user will have outdated copies.
// Note: Considered undefined behavior to call when game window is minimized.
bool
uvkRebuildSwapChain(const void* const                           os_info,
                    const VkPhysicalDevice                      physical_device,
                    const VkDevice                              logical_device,
                    const VkInstance                            instance,
                    const uGameWindowProperties* const restrict game_window_props,
                    const u32                                   graphics_queue_family_index,
                    const u32                                   present_queue_family_index,
                    const VkPipelineShaderStageCreateInfo* const restrict shader_stage_cis,
                    const u32                                             shader_stage_ci_count,
                    VkSwapchainKHR                                        swapchain_to_rebuild,
                    VkSurfaceKHR                                          surface_to_rebuild,
                    VkSurfaceCapabilitiesKHR    surface_cababilities_to_rebuild,
                    VkSurfaceFormatKHR          surface_format_to_rebuild,
                    VkPresentModeKHR            present_mode_to_rebuild,
                    VkImage* const restrict     images_to_rebuild,
                    VkImageView* const restrict image_views_to_rebuild,
                    VkRenderPass                render_pass_to_rebuild,
                    VkPipeline                  graphics_pipeline_to_rebuild,
                    VkFramebuffer               frame_buffer_to_rebuild,
                    VkCommandBuffer             command_buffer_to_rebuild,
                    u32* const restrict         images_rebuilt,
                    u32* const restrict         image_views_rebuilt)
{
    // [ cfarvin::TODO ] uAssertMsg_v
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");

    vkDeviceWaitIdle(logical_device);

    // [ cfarvin::TODO ] Destroy old surface
    // Rebuild surface.

    VkSurfaceKHR new_surface = { 0 };

#if _WIN32
    const uWin32Info* win32_info = (uWin32Info*)os_info;
    uvkCreateWin32Surface(instance, win32_info->window, win32_info->instance, &new_surface);
#endif

    // Get new surface capabilities.
    VkSurfaceCapabilitiesKHR new_surface_capabilities = { 0 };
    if (false == uvkGetSurfaceCapabilities(physical_device, new_surface, &new_surface_capabilities))
    {
        uError("[ vulkan ] Unable to rebuild swapchain with new surface capabilities.\n");
        return false;
    }

    // Get new surface formats.
    VkSurfaceFormatKHR* new_surface_formats      = NULL;
    u32                 new_surface_format_count = 0;
    if (false == uvkGetSurfaceFormats(physical_device,
                                      new_surface,
                                      &new_surface_formats,
                                      &new_surface_format_count))
    {
        uError("[ vulkan ] Unable to rebuild swapchainwith new surface formats.\n");
        return false;
    }

    // Get new surface present modes.
    VkPresentModeKHR* new_present_modes      = NULL;
    u32               new_present_mode_count = 0;
    if (false == uvkGetPresentModes(physical_device,
                                    new_surface,
                                    &new_present_modes,
                                    &new_present_mode_count))
    {
        uError("[ vulkan ] Unable to rebuild swapchain with new present modes.\n");
        return false;
    }

    // Get new surface format index.
    u32 new_surface_format_index = 0;
    if (false == uvkSelectSurfaceFormat(new_surface_formats,
                                        new_surface_format_count,
                                        VK_FORMAT_B8G8R8A8_SRGB,
                                        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                                        &new_surface_format_index))
    {
        // Note: If this occurs, option to fall back to the first available index.
        uError("[ vulkan ] Unable to select suitable surface format.\n");
        return false;
    }

    // Get new present mode index.
    u32              new_present_mode_index      = 0;
    VkPresentModeKHR acceptable_present_modes[2] = {
        VK_PRESENT_MODE_MAILBOX_KHR, // Preferred.
        VK_PRESENT_MODE_FIFO_KHR     // Acceptable.
    };

    bool suitable_present_mode_found = false;
    for (size_t present_mode_idx = 0; present_mode_idx < 2; present_mode_idx++)
    {
        if (true == uvkSelectPresentMode(new_present_modes,
                                         new_present_mode_count,
                                         acceptable_present_modes[present_mode_idx],
                                         &new_present_mode_index))
        {
            suitable_present_mode_found = true;
            break;
        }
    }

    if (false == suitable_present_mode_found)
    {
        // Note: If this occurs, option to fall back to the first available index.
        uError("[ vulkan ] Unable to select suitable present mode.\n");
        return false;
    }

    // Validate new surface and old device compatibility.
    VkBool32 has_present_capability = false;
    VkResult new_surface_compatible = vkGetPhysicalDeviceSurfaceSupportKHR(
      physical_device,
      present_queue_family_index,
      new_surface,
      &has_present_capability);

    if (VK_SUCCESS != new_surface_compatible)
    {
        uError("[ vulkan ] Unable to rebuild a compatible surface.\n");
        return false;
    }

    if (false == has_present_capability)
    {
        uError("[ vulkan ] Unable to rebuild a valid present surface.\n");
        return false;
    }

    VkSurfaceFormatKHR new_surface_format = new_surface_formats[new_surface_format_index];
    VkPresentModeKHR   new_present_mode   = new_present_modes[new_present_mode_index];
    free(new_surface_formats);
    free(new_present_modes);

    // Rebuild swapchain (swapchain, images & image views)
    VkSwapchainKHR new_swapchain = { 0 };
    /* VkImage*       new_images    = NULL; */ // [ cfarvin::NOTE ] Removed when updating Swapchain creation
    /* VkImageView*   new_image_views = NULL; */ // [ cfarvin::NOTE ] Removed when uvkCreateSwapchain was updated
    u32 new_image_count = 0;

    // uDestroyVulkanImageGroup();
    // uDestroyVulkanCommandBuffers();

    // [ cfarvin::REMOVE ]
    // uResetVulkanRenderInfo();

    // [ cfarvin::NOTE ] I REMOVED THE IMAGE VIEWS PTR HERE
    uvkCreateSwapchain(logical_device,
                       new_surface,
                       new_surface_capabilities,
                       new_surface_format,
                       new_present_mode,
                       graphics_queue_family_index,
                       present_queue_family_index,
                       game_window_props,
                       &new_swapchain,
                       &new_image_count);

    // [ cfarvin::TODO ] Correctness
    //
    // We need to ensure that the values commented out below end up being assigned by
    // either this function or, more ideally, uCreateVulkanSwapchainCore.
    //
    // Fill in new surface info values.
    // uVulkanSurfaceInfo* surface_info         = (uVulkanSurfaceInfo*)uGetVulkanSurfaceInfo();
    // *((VkSurfaceKHR*)&surface_info->surface) = new_surface;
    // *((VkSurfaceCapabilitiesKHR*)&surface_info->surface_capabilities) = new_surface_capabilities;
    // *((VkSurfaceFormatKHR*)&surface_info->surface_format)             = new_surface_format;
    // *((VkPresentModeKHR*)&surface_info->present_mode)                 = new_present_mode;

    // // [ cfarvin::TODO ] Framebuffers?
    // // Fill in new image group values.
    // uVulkanImageGroup* image_group              = (uVulkanImageGroup*)uGetVulkanImageGroup();
    // *((VkImage**)&image_group->images)          = new_images;
    // *((VkImageView**)&image_group->image_views) = new_image_views;
    // *((u32*)&image_group->num_images)           = new_image_count;
    // *((VkSwapchainKHR*)&image_group->swapchain) = new_swapchain;

    // /* uRebuidlRenderTools(render_tools); */
    // render_tools->swapchain = new_swapchain;

    // uvkCreateImageViews();
    // uCreateVulkanRenderPass();
    // uCreateVulkanGraphicsPipeline(shader_stage_ci_count, shader_stage_cis);
    // uCreateVulkanFrameBuffers();
    // uCreateVulkanCommandBuffers();

    return true;
}

bool
uvkCreateGraphicsCommandPool(const VkDevice           logical_device,
                             const u32                graphics_queue_family_index,
                             VkCommandPoolCreateFlags flags,
                             VkCommandPool*           return_command_pool)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != return_command_pool, "[ vulkan ] VkCommandPool ptr must be non null.\n");

    VkCommandPoolCreateInfo command_pool_ci = default_command_pool_ci;
    command_pool_ci.queueFamilyIndex        = graphics_queue_family_index;
    command_pool_ci.flags                   = flags;

    VkResult result = vkCreateCommandPool(logical_device,
                                          &command_pool_ci,
                                          NULL,
                                          return_command_pool);
    if (result != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to create command pool.\n");
        return false;
    }

    return true;
}

void
uvkDestroyCommandPool(const VkDevice logical_device, VkCommandPool command_pool)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != command_pool, "[ vulkan ] VkCommandPool must be non null.\n");

    vkDestroyCommandPool(logical_device, command_pool, NULL);
}

bool
uvkCreateImageViews(const VkDevice                logical_device,
                    const VkSurfaceFormatKHR      surface_format,
                    const VkImage* const restrict images,
                    const u32                     image_count,
                    const VkImageViewCreateFlags  flags,
                    const VkImageViewType         view_type,
                    const VkComponentMapping      component_mapping,
                    const VkImageSubresourceRange sub_resource_range,
                    VkImageView** restrict        return_image_views)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != images, "[ vulkan ] VkImage ptr must be non null.\n");
    uAssertMsg_v(NULL != *images, "[ vulkan ] VkImage must be non null.\n");
    uAssertMsg_v(0 != image_count, "[ vulkan ] Image count must be non zero.\n");
    uAssertMsg_v(NULL != return_image_views, "[ vulkan ] VkImageView ptr must be non null.\n");
    uAssertMsg_v(NULL == *return_image_views,
                 "[ vulkan ] VkImageView must be null; will be overwritten\n");

    size_t alloc_bytes  = sizeof(VkImageView) * image_count;
    *return_image_views = (VkImageView*)malloc(alloc_bytes);
    if (NULL == return_image_views)
    {
        uError("[ vulkan ] Unable to allocate memory for image views.\n");
        return false;
    }

    VkImageViewCreateInfo image_view_ci = default_image_view_ci;
    image_view_ci.flags                 = flags;
    image_view_ci.viewType              = view_type;
    image_view_ci.format                = surface_format.format;
    image_view_ci.components            = component_mapping;
    image_view_ci.subresourceRange      = sub_resource_range;

    VkResult     result;
    VkImageView* image_view_array = return_image_views[0];
    for (u32 image_idx = 0; image_idx < image_count; image_idx++)
    {
        // Create image view
        image_view_ci.image = images[image_idx];
        result              = vkCreateImageView(logical_device,
                                   &image_view_ci,
                                   NULL,
                                   &image_view_array[image_idx]);

        if (VK_SUCCESS != result)
        {
            uError("[ vulkan ] Unable to create image view.\n");
            return false;
        }
    }

    return true;
}

void
uvkDestroyImageViews(const VkDevice logical_device,
                     VkImageView**  image_views,
                     VkImage**      images,
                     const u32      image_view_count)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != image_views, "[ vulkan ] VkImageView ptr must be non null.\n");
    uAssertMsg_v(NULL != images, "[ vulkan ] VkImage ptr must be non null.\n");
    uAssertMsg_v(0 != image_view_count, "[ vulkan ] Image view count must be non zero.\n");

    VkImageView  image_view       = NULL;
    VkImageView* image_view_array = image_views[0];
    for (size_t image_view_iter = 0; image_view_iter < image_view_count; image_view_iter++)
    {
        // Delete image views.
        image_view = image_view_array[image_view_iter];
        if (NULL != image_view)
        {
            vkDestroyImageView(logical_device, image_view, NULL);
        }
        else
        {
            uWarning("Unable to destroy NULL VkImageView at index %zd.\n", image_view_iter);
        }
    }

    free(*images);
    *images = NULL;

    free(*image_views);
    *image_views = NULL;
}

bool
uvkGetSwapchainImages(const VkDevice       logical_device,
                      const VkSwapchainKHR swapchain,
                      u32                  requested_image_count,
                      VkImage**            return_images,
                      u32*                 return_image_count)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != swapchain, "[ vulkan ] VkSwapchainKHR must be non null.\n");
    uAssertMsg_v(0 != requested_image_count,
                 "[ vulkan ] Requested image count must be non zero.\n");
    uAssertMsg_v(NULL != return_images, "[ vulkan ] VkImage array must be non null.\n");
    uAssertMsg_v(NULL == *return_images, "[ vulkan ] VkImage must be null; will be overwritten.\n");
    uAssertMsg_v(NULL != return_image_count,
                 "[ vulkan ] Return image count ptr must be non null.\n");
    uAssertMsg_v(0 == *return_image_count,
                 "[ vulkan ] Return image count must be zero; will be overwritten.\n");

    *return_image_count = requested_image_count;
    VkResult result = vkGetSwapchainImagesKHR(logical_device, swapchain, return_image_count, NULL);
    if (VK_SUCCESS != result)
    {
        uError("[ vulkan ] Unable to obtain swapchain image count.\n");
        free(*return_images);
        return false;
    }

    if (0 == requested_image_count)
    {
        uError("[ vulkan ] Swapchain reports no images.\n");
        free(*return_images);
        return false;
    }

    if (*return_image_count != requested_image_count)
    {
        uWarning("[ vulkan ] Swapchain image count was overriden by driver.\n");
        *return_image_count = requested_image_count;
    }

    *return_images = (VkImage*)calloc(*return_image_count, sizeof(VkImage));
    if (NULL == return_images)
    {
        uError("[ vulkan ] Unable to allocate VkImages.\n");
        return false;
    }

    result = vkGetSwapchainImagesKHR(logical_device, swapchain, return_image_count, *return_images);
    if (VK_SUCCESS != result)
    {
        uError("[ vulkan ] Unable to set swapchain image count handle(s).\n");
        return false;
    }

    return true;
}

bool
uvkCreateSwapchain(const VkDevice                              logical_device,
                   const VkSurfaceKHR                          surface,
                   const VkSurfaceCapabilitiesKHR              surface_capabilities,
                   const VkSurfaceFormatKHR                    surface_format,
                   const VkPresentModeKHR                      present_mode,
                   const u32                                   graphics_queue_family_index,
                   const u32                                   present_queue_family_index,
                   const uGameWindowProperties* const restrict game_window_props,
                   VkSwapchainKHR* const restrict              return_swapchain,
                   u32* const restrict                         return_preferred_image_count)

{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != game_window_props, "[ vulkan ] uGameWindowProperties must be non null.\n");
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != surface, "[ vulkan ] VkSurfaceKHR must be non null.\n");
    uAssertMsg_v(NULL != return_swapchain, "[ vulkan ] Return VkSwapchinKHR  must be non null.\n");
    uAssertMsg_v(NULL != return_preferred_image_count,
                 "[ vulkan ] Return VkImage array must be non null.\n");

    VkResult result;

    // Determine swap extent.
    VkExtent2D surface_extent = { 0 };
    uvkGetSurfaceExtent(surface_capabilities, game_window_props, &surface_extent);

    // Determine swapchain image capacity.
    u32 min_image_count           = surface_capabilities.minImageCount;
    u32 max_image_count           = surface_capabilities.maxImageCount;
    *return_preferred_image_count = min_image_count + 1;

    // Ensure designated image count is within bounds.
    bool image_count_unlimited = (max_image_count == 0);
    if (false == image_count_unlimited && (*return_preferred_image_count > max_image_count))
    {
        *return_preferred_image_count = max_image_count;
    }

    // Determine number of unique queues.
    u32 unique_queues_found = 0;
    uvkCountUniqueQueueFamilies(graphics_queue_family_index,
                                present_queue_family_index,
                                &unique_queues_found);

    VkSwapchainCreateInfoKHR swapchain_ci = default_swapchain_ci;
    swapchain_ci.surface                  = surface;
    swapchain_ci.minImageCount            = *return_preferred_image_count;
    swapchain_ci.imageFormat              = surface_format.format;
    swapchain_ci.imageColorSpace          = surface_format.colorSpace;
    swapchain_ci.imageExtent              = surface_extent;
    swapchain_ci.imageArrayLayers         = 1;
    swapchain_ci.preTransform             = surface_capabilities.currentTransform;
    swapchain_ci.presentMode              = present_mode;

    // Will be needed if and only if the queue families are not unique.
    u32 queue_family_indices_array[2] = { graphics_queue_family_index, present_queue_family_index };
    if (1 == unique_queues_found)
    {
        // Queues were unique/identical.
        swapchain_ci.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_ci.queueFamilyIndexCount = 0; // Zero is valid for VK_SHARING_MODE_EXCLUSIVE.
        swapchain_ci.pQueueFamilyIndices   = NULL;
    }
    else
    {
        // Queues were not unique/identical.
        swapchain_ci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = unique_queues_found;
        swapchain_ci.pQueueFamilyIndices   = &(queue_family_indices_array[0]);
    }

    result = vkCreateSwapchainKHR(logical_device, &swapchain_ci, NULL, return_swapchain);
    if (result != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to create swapchain.\n");
        return false;
    }

    return true;
}

void
uvkDestroySwapchain(const VkDevice logical_device, VkSwapchainKHR swapchain)
{
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");
    uAssertMsg_v(NULL != swapchain, "[ vulkan ] VkSwapchainKHR must be non null.\n");

    vkDestroySwapchainKHR(logical_device, swapchain, NULL);
    swapchain = NULL;
}

void
uvkGetSurfaceExtent(const VkSurfaceCapabilitiesKHR              surface_capabilities,
                    const uGameWindowProperties* restrict const game_window_props,
                    VkExtent2D* const restrict                  surface_extent)
{
    // Note: Some window managers use u32_MAX as a magic value which indicates
    // that the extent must be manually set up.
    u32 u32_MAX = ~(u32)0;
    if (surface_capabilities.currentExtent.width != u32_MAX &&
        surface_capabilities.currentExtent.height != u32_MAX)
    {
        surface_extent->width  = surface_capabilities.currentExtent.width;
        surface_extent->height = surface_capabilities.currentExtent.height;
    }
    else
    {
        surface_extent->width  = game_window_props->width;
        surface_extent->height = game_window_props->height;
    }

    // Ensure we do not exceed maximums
    if (surface_extent->width > surface_capabilities.maxImageExtent.width)
    {
        uError("[ vulkan ] Calculated extent width exceeded surface "
               "capabiility; capped.\n");
        surface_extent->width = surface_capabilities.maxImageExtent.width;
    }

    if (surface_extent->height > surface_capabilities.maxImageExtent.height)
    {
        uError("[ vulkan ] Calculated extent height exceeded surface "
               "capabiility; capped.\n");
        surface_extent->height = surface_capabilities.maxImageExtent.height;
    }

    // Ensure we do not fall below minimums
    if (surface_extent->width < surface_capabilities.minImageExtent.width)
    {
        uError("[ vulkan ] Calculated extent width fell below surface "
               "capabiility; capped.\n");
        surface_extent->width = surface_capabilities.minImageExtent.width;
    }

    if (surface_extent->height < surface_capabilities.minImageExtent.height)
    {
        uError("[ vulkan ] Calculated extent height fell below surface "
               "capabiility; capped.\n");
        surface_extent->height = surface_capabilities.minImageExtent.height;
    }
}

// Note: VkExtensionProperties ptr and return extension count ptr may be NULL. This
// function is used by uvkValidateExtensions, wich can query for either device or
// instance extensions by leaving one or neither of these pointers NULL.
bool
uvkGetDeviceExtensions(const VkPhysicalDevice                 physical_device,
                       VkExtensionProperties** const restrict return_extension_properties,
                       u32* const                             return_extension_count)
{
    uAssertMsg_v(NULL != return_extension_properties,
                 "[ vulkan ] VkExtensionProperties ptr must be non null.\n");

    VkResult success = vkEnumerateDeviceExtensionProperties(physical_device,
                                                            NULL,
                                                            return_extension_count,
                                                            NULL);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to enumerate device extension properties.\n");
        return false;
    }

    if (0 == *return_extension_count)
    {
        uError("[ vulkan ] There were no device extensions available on this device.\n");
        return false;
        ;
    }

    size_t extension_properties_bytes = *return_extension_count * sizeof(VkExtensionProperties);
    *return_extension_properties      = (VkExtensionProperties*)malloc(extension_properties_bytes);
    if (NULL == *return_extension_properties)
    {
        uError("[ vulkan ] Unable to allocate memory.\n");
        return false;
    }

    success = vkEnumerateDeviceExtensionProperties(physical_device,
                                                   NULL,
                                                   return_extension_count,
                                                   *return_extension_properties);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to enumerate device extension properties.\n");
        return false;
    }

    return true;
}

bool
uvkGetInstanceExtensions(VkExtensionProperties** const restrict return_extension_properties,
                         u32* const                             return_extension_count)
{
    uAssertMsg_v(NULL != return_extension_properties,
                 "[ vulkan ] VkExtensionProperties ptr must be non null.\n");

    VkResult success = vkEnumerateInstanceExtensionProperties(NULL, return_extension_count, NULL);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to enumerate instance extension properties.\n");
        return false;
    }

    if (0 == *return_extension_count)
    {
        uError("[ vulkan ] There were no instance extensions available on this device.\n");
        return false;
    }

    size_t extension_properties_bytes = *return_extension_count * sizeof(VkExtensionProperties);
    *return_extension_properties      = (VkExtensionProperties*)malloc(extension_properties_bytes);
    if (NULL == *return_extension_properties)
    {
        uError("[ vulkan ] Unable to allocate memory.\n");
        return false;
    }

    success = vkEnumerateInstanceExtensionProperties(NULL,
                                                     return_extension_count,
                                                     *return_extension_properties);
    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Unable to enumerate instance extension properties.\n");
        return false;
    }

    return true;
}

bool
uvkGetInstanceLayers(VkLayerProperties** const restrict return_layer_properties,
                     u32* const                         return_layer_count)
{
    uAssertMsg_v(NULL != return_layer_properties,
                 "[ vulkan ] VkExtensionProperties ptr must be non null.\n");

    VkResult success = vkEnumerateInstanceLayerProperties(return_layer_count, NULL);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to enumerate instance layer properties.\n");
        return false;
    }

    if (0 == *return_layer_count)
    {
        uError("[ vulkan ] There were no instance layers available on this device.\n");
        return false;
    }

    size_t layer_properties_bytes = *return_layer_count * sizeof(VkLayerProperties);
    *return_layer_properties      = (VkLayerProperties*)malloc(layer_properties_bytes);
    if (NULL == *return_layer_properties)
    {
        uError("[ vulkan ] Unable to allocate memory.\n");
        return false;
    }

    success = vkEnumerateInstanceLayerProperties(return_layer_count, *return_layer_properties);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to enumerate instance layer properties.\n");
        return false;
    }

    return true;
}

size_t
uvkValidateUserExtensions(const char** const restrict                 user_extensions,
                          const size_t                                user_extension_count,
                          const VkExtensionProperties* const restrict extension_properties,
                          const size_t                                extension_property_count)
{
    uAssertMsg_v(NULL != user_extensions, "[ vulkan ] User extension name ptr must be non null.\n");
    uAssertMsg_v(0 != user_extension_count, "[ vulkan ] User extension count must be non zero.\n");
    uAssertMsg_v(NULL != extension_properties,
                 "[ vulkan ] VkExtensionProperties ptr must be non null.\n");
    uAssertMsg_v(0 != extension_property_count,
                 "[ vulkan ] Extension property count must be non zero.\n");

    // Validate extensions by name.
    u8          match_found          = 0;
    size_t      validated_extensions = 0;
    const char* user_ext_name        = NULL;
    const char* ext_prop_name        = NULL;
    for (size_t user_ext_iter = 0; user_ext_iter < user_extension_count; user_ext_iter++)
    {
        match_found = 0;
        for (size_t ext_prop_iter = 0; ext_prop_iter < extension_property_count; ext_prop_iter++)
        {
            ext_prop_name = extension_properties[ext_prop_iter].extensionName;
            user_ext_name = user_extensions[user_ext_iter];
            uAssertMsg_v(NULL != user_ext_name,
                         "[ vulkan ] User extension names must be non NULL.\n");
            if (NULL == user_ext_name)
            {
                uError_v("[ vulkan ] The %zdth index of the user extension names was null.\n",
                         user_ext_iter);
                continue;
            }

            if (0 == strcmp(ext_prop_name, user_ext_name))
            {
                match_found = 1;
                break;
            }
        }

        if (1 == match_found)
        {
            validated_extensions++;
        }
        else
        {
            uError("[ vulkan ] The extension %s is unavailable on this device.\n",
                   user_extensions[user_ext_iter]);
        }
    }

    return validated_extensions;
}

size_t
uvkValidateUserLayers(const char** const restrict             user_layers,
                      const size_t                            user_layer_count,
                      const VkLayerProperties* const restrict layer_properties,
                      const size_t                            layer_property_count)
{
    uAssertMsg_v(NULL != user_layers, "[ vulkan ] User layer name ptr must be non null.\n");
    uAssertMsg_v(0 != user_layer_count, "[ vulkan ] User layer count must be non zero.\n");
    uAssertMsg_v(NULL != layer_properties, "[ vulkan ] VkLayerProperties ptr must be non null.\n");
    uAssertMsg_v(0 != layer_property_count, "[ vulkan ] Layer property count must be non zero.\n");

    // Validate layers by name.
    u8          match_found      = 0;
    size_t      validated_layers = 0;
    const char* user_layer_name  = NULL;
    const char* layer_prop_name  = NULL;
    for (size_t user_layer_iter = 0; user_layer_iter < user_layer_count; user_layer_iter++)
    {
        match_found = 0;
        for (size_t layer_prop_iter = 0; layer_prop_iter < layer_property_count; layer_prop_iter++)
        {
            layer_prop_name = layer_properties[layer_prop_iter].layerName;
            user_layer_name = user_layers[user_layer_iter];
            uAssertMsg_v(NULL != user_layer_name,
                         "[ vulkan ] User layer names must be non NULL.\n");
            if (NULL == user_layer_name)
            {
                uError_v("[ vulkan ] The %zdth index of the user layer names was null.\n",
                         user_layer_iter);
                continue;
            }

            if (0 == strcmp(layer_prop_name, user_layer_name))
            {
                match_found = 1;
                break;
            }
        }

        // If a match is found for this layer name, set the corresponding index in the
        // validation array.
        if (1 == match_found)
        {
            validated_layers++;
        }
        else
        {
            uError("[ vulkan ] The layer %s is unavailable on this device.\n",
                   user_layers[user_layer_iter]);
        }
    }

    return validated_layers;
}

bool
uvkCreateLogicalDevice(const VkPhysicalDevice          physical_device,
                       const VkSurfaceKHR              surface,
                       const char** const restrict     user_validation_layer_names,
                       const size_t                    user_validation_layer_name_count,
                       const char** const restrict     user_device_extension_names,
                       const size_t                    user_device_extension_name_count,
                       VkDevice*                       return_logical_device,
                       uvkActiveQueues* const restrict return_active_queues)
{
    uAssertMsg_v(NULL != physical_device, "[ vulkan ] VkPhysicalDevice must be non null.\n");
    uAssertMsg_v(NULL != surface, "[ vulkan ] VkSurfaceKHR must be non null.\n");
    uAssertMsg_v(NULL != return_logical_device,
                 "[ vulkan ] Return VkDevice ptr must be non null.\n");
    uAssertMsg_v(NULL != return_active_queues,
                 "[ vulkan ] The uvkActiveQueues ptr must be non null.\n");

    if (user_validation_layer_name_count)
    {
        uAssertMsg_v(
          user_validation_layer_names && *user_validation_layer_names,
          "[ vulkan ] If the instance layer count is non zero, the names ptr must be non null\n");
    }

    if (user_device_extension_name_count)
    {
        uAssertMsg_v(
          user_device_extension_names && *user_device_extension_names,
          "[ vulkan ] If the device extension count is non zero, the names ptr must be non null\n");
    }

    u32 graphics_queue_family_index = 0;
    u32 present_queue_family_index  = 0;
    if (false == uvkGetQueueFamilyIndices(physical_device,
                                          surface,
                                          &graphics_queue_family_index,
                                          &present_queue_family_index))
    {
        uError("[ vulkan ] Unable to get queue family properties.\n");
        return false;
    }

    // Create device queue create infos based on unique queue indices.
    VkDeviceQueueCreateInfo* device_queue_cis = NULL;
    if (false == uvkGetDeviceQueueCreateInfos(graphics_queue_family_index,
                                              present_queue_family_index,
                                              default_device_queue_ci,
                                              &device_queue_cis,
                                              &(return_active_queues->active_queue_family_count)))
    {
        uError("[ vulkan ] Unable to get device queue create info(s).\n");
        return false;
    }

    // Assign the queue family indices.
    return_active_queues->graphics_queue_family_index = graphics_queue_family_index;
    return_active_queues->present_queue_family_index  = present_queue_family_index;

    // [ cfarvin::TODO ] Check correctness.
    //
    // Added the vkGetPhysicalDeviceFeatures call a while after the original
    // implementation. I think this was an oversight originally; let's be sure.
    // If there are sudden errors in this part of the code; this could be why.
    VkPhysicalDeviceFeatures physical_device_features = { 0 };
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    VkDeviceCreateInfo logical_device_ci   = default_device_ci;
    logical_device_ci.pQueueCreateInfos    = device_queue_cis;
    logical_device_ci.queueCreateInfoCount = return_active_queues->active_queue_family_count;
    logical_device_ci.pEnabledFeatures     = &physical_device_features;

    if (0 != user_validation_layer_name_count)
    {
        logical_device_ci.enabledLayerCount   = user_validation_layer_name_count;
        logical_device_ci.ppEnabledLayerNames = user_validation_layer_names;
    }

    logical_device_ci.enabledExtensionCount   = user_device_extension_name_count;
    logical_device_ci.ppEnabledExtensionNames = user_device_extension_names;

    VkResult device_creation_success = vkCreateDevice(physical_device,
                                                      &logical_device_ci,
                                                      NULL,
                                                      return_logical_device);

    if (device_queue_cis)
    {
        free(device_queue_cis);
    }

    if (device_creation_success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to create logical device.\n");
        return false;
    }

    vkGetDeviceQueue(*return_logical_device,
                     graphics_queue_family_index,
                     0, // Use first index in this family.
                     &(return_active_queues->graphics_queue));

    vkGetDeviceQueue(*return_logical_device,
                     present_queue_family_index,
                     0, // Use first index in this family.
                     &(return_active_queues->present_queue));

    return true;
}

void
uvkDestroyLogicalDevice(VkDevice logical_device)
{
    vkDestroyDevice(logical_device, NULL);
}

bool
uvkSelectPresentMode(const VkPresentModeKHR* const restrict present_modes,
                     const u32                              present_mode_count,
                     VkPresentModeKHR                       desired_present_mode,
                     u32* const restrict                    return_present_mode_index)
{
    uAssertMsg_v(NULL != present_modes, "[ vulkan ] VkPresentModeKHR ptr must be non null.\n");
    uAssertMsg_v(0 != present_mode_count, "[ vulkan ] Present mode count must be non zero.\n");
    uAssertMsg_v(NULL != return_present_mode_index,
                 "[ vulkan ] Return index ptr must be non null.\n");

    bool desired_present_mode_found = false;
    for (u16 present_idx = 0; present_idx < present_mode_count; present_idx++)
    {
        if (present_modes[present_idx] == desired_present_mode)
        {
            *return_present_mode_index = present_idx;
            desired_present_mode_found = true;
            break;
        }
    }

    return desired_present_mode_found;
}

bool
uvkSelectSurfaceFormat(const VkSurfaceFormatKHR* const restrict surface_formats,
                       const u32                                surface_format_count,
                       const VkFormat                           desired_format,
                       const VkColorSpaceKHR                    desired_color_space,
                       u32* const restrict                      return_surface_format_index)
{
    uAssertMsg_v(NULL != surface_formats, "[ vulkan ] VkSurfaceFormatKHR ptr must be non null.\n");
    uAssertMsg_v(0 != surface_format_count, "[ vulkan ] Surface format count must be non zero.\n");
    uAssertMsg_v(NULL != return_surface_format_index,
                 "[ vulkan ] Return index ptr must be non null.\n");

    bool optimal_format_found = false;
    for (u16 format_idx = 0; format_idx < surface_format_count; format_idx++)
    {
        if (surface_formats[format_idx].format == desired_format &&
            surface_formats[format_idx].colorSpace == desired_color_space)
        {
            *return_surface_format_index = format_idx;
            optimal_format_found         = true;
            break;
        }
    }

    return optimal_format_found;
}

bool
uvkGetPresentModes(const VkPhysicalDevice            physical_device,
                   const VkSurfaceKHR                surface,
                   VkPresentModeKHR** const restrict return_present_modes,
                   u32* const restrict               return_present_mode_count)
{
    VkResult success = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                                 surface,
                                                                 return_present_mode_count,
                                                                 NULL);
    if (success != VK_SUCCESS || 0 == *return_present_mode_count)
    {
        return false;
    }

    const size_t array_bytes = *return_present_mode_count * sizeof(VkPresentModeKHR);
    *return_present_modes    = (VkPresentModeKHR*)malloc(array_bytes);
    if (NULL == return_present_modes)
    {
        return false;
    }

    const u32 requested_num_present_modes = *return_present_mode_count;
    success = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                        surface,
                                                        return_present_mode_count,
                                                        *return_present_modes);
    if (success != VK_SUCCESS)
    {
        if (*return_present_modes)
        {
            free(*return_present_modes);
        }

        return false;
    }

    if (requested_num_present_modes != *return_present_mode_count)
    {
        if (*return_present_modes)
        {
            free(*return_present_modes);
        }

        return false;
    }

    return true;
}

// Note: Allocates memory that is not freed in this scope.
// Note: We do not log any error messages here as it is called in a loop where some
//       invalid returns from vulkan functions are possible but not fatal.
bool
uvkGetSurfaceFormats(const VkPhysicalDevice              physical_device,
                     const VkSurfaceKHR                  surface,
                     VkSurfaceFormatKHR** const restrict return_surface_formats,
                     u32* const restrict                 return_surface_format_count)
{
    uAssertMsg_v(NULL != return_surface_formats,
                 "[ vulkan ] VkSurfaceFormatKHR ptr ptr must be non null.\n");
    uAssertMsg_v(NULL != return_surface_format_count,
                 "[ vulkan ] Return surface format count ptr must be non null.\n");

    VkResult success = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                                            surface,
                                                            return_surface_format_count,
                                                            NULL);
    if (success != VK_SUCCESS || 0 == *return_surface_format_count)
    {
        return false;
    }

    const size_t array_bytes = *return_surface_format_count * sizeof(VkSurfaceFormatKHR);
    *return_surface_formats  = (VkSurfaceFormatKHR*)malloc(array_bytes);
    if (NULL == return_surface_formats)
    {
        return false;
    }

    const u32 requested_surface_format_count = *return_surface_format_count;
    success                                  = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                                   surface,
                                                   return_surface_format_count,
                                                   *return_surface_formats);
    if (success != VK_SUCCESS)
    {
        if (*return_surface_formats)
        {
            free(*return_surface_formats);
        }

        return false;
    }

    if (requested_surface_format_count != *return_surface_format_count)
    {
        if (*return_surface_formats)
        {
            free(*return_surface_formats);
        }

        return false;
    }

    return true;
}

bool
uvkGetSurfaceCapabilities(const VkPhysicalDevice                   physical_device,
                          const VkSurfaceKHR                       surface,
                          VkSurfaceCapabilitiesKHR* const restrict return_surface_capabilities)
{
    uAssertMsg_v(NULL != return_surface_capabilities,
                 "[ vulkan ] VkSurfaceCapabilitiesKHR ptr must be non null.\n");

    VkResult success = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                                 surface,
                                                                 return_surface_capabilities);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Could not attain surface capabilities.\n");
        return false;
    }

    return true;
}

bool
uvkGetDeviceQueueCreateInfos(const u32                                graphics_queue_family_index,
                             const u32                                present_queue_family_index,
                             const VkDeviceQueueCreateInfo            device_queue_ci,
                             VkDeviceQueueCreateInfo** const restrict return_device_queue_cis,
                             u32* const restrict                      return_unique_queue_count)
{
    uAssertMsg_v(NULL != return_device_queue_cis,
                 "[ vulkan ] The VkDeviceQueueCreateinfo ptr must be non null.\n");
    uAssertMsg_v(0 == *return_device_queue_cis,
                 "[ vulkan ] The return create info array must be zeroed; will be overwritten.\n");
    uAssertMsg_v(NULL != return_unique_queue_count,
                 "[ vulkan ] Return unique queue count must be non null.\n");
    uAssertMsg_v(0 == *return_unique_queue_count,
                 "[ vulkan ] Return unique queue count must be zero; will be overwritten.\n");

    uvkCountUniqueQueueFamilies(graphics_queue_family_index,
                                present_queue_family_index,
                                return_unique_queue_count);

    if (0 == *return_unique_queue_count)
    {
        uError("[ vulkan ] Unable to determine the number of unique queues.\n");
        return false;
    }

    *return_device_queue_cis = (VkDeviceQueueCreateInfo*)calloc(*return_unique_queue_count,
                                                                sizeof(VkDeviceQueueCreateInfo));
    if (NULL == *return_device_queue_cis)
    {
        uError("[ vulkan ] Unable to allocate memory.\n");
        return false;
    }

    u32 queue_family_indices[2] = { graphics_queue_family_index, present_queue_family_index };

    for (u32 queue_family_idx = 0; queue_family_idx < *return_unique_queue_count;
         queue_family_idx++)
    {
        *return_device_queue_cis[queue_family_idx] = device_queue_ci;
        (*return_device_queue_cis[queue_family_idx]).queueFamilyIndex =
          queue_family_indices[queue_family_idx];
    }

    return true;
}

void
uvkCountUniqueQueueFamilies(const u32     graphics_queue_family_index,
                            const u32     present_queue_family_index,
                            u32* restrict return_unique_queue_family_count)
{
    uAssertMsg_v(return_unique_queue_family_count,
                 "[ vulkan ] Number of unique queues found ptr must be non null.\n");
    uAssertMsg_v(!(*return_unique_queue_family_count),
                 "[ vulkan ] Number of unique queues found must be zero; will "
                 "be overwritten.\n");

// [ cfarvin::REVISIT ] Do we need to enforace a max unique queue count? Could a reasonable
//                      user need control of this for this application? Is it used elsewhere?
//                      If so, then paramaterize it.
#define MAX_UNIQUE_QUEUE_COUNT 2
    u32 queue_array[MAX_UNIQUE_QUEUE_COUNT];
    queue_array[0] = graphics_queue_family_index;
    queue_array[1] = present_queue_family_index;

    // Consider the first queue family index to be unique.
    *return_unique_queue_family_count = 1;

    // Find other unique queue family indices, if they exist.
    for (size_t a_iter = 0; a_iter < MAX_UNIQUE_QUEUE_COUNT; a_iter++)
    {
        bool is_unique = true;
        u32  value_a   = queue_array[a_iter];
        for (size_t b_iter = 0; b_iter < MAX_UNIQUE_QUEUE_COUNT; b_iter++)
        {
            // Don't compare the same index.
            if (a_iter == b_iter)
            {
                continue;
            }

            u32 value_b = queue_array[b_iter];
            if (value_a == value_b)
            {
                is_unique = false;
                break;
            }
        }

        if (is_unique)
        {
            (*return_unique_queue_family_count)++;
        }
    }
#undef MAX_UNIQUE_QUEUE_COUNT
}

bool
uvkGetQueueFamilyProperties(const VkPhysicalDevice                   physical_device,
                            VkQueueFamilyProperties** const restrict return_queue_family_props,
                            u32* const                               return_queue_family_prop_count)
{
    uAssertMsg_v(NULL != physical_device, "[ vulkan ] VkPhysical device must be valid.\n");
    uAssertMsg_v(NULL != return_queue_family_props,
                 "[ vulkan ] VkQueueFamilyProperties ptr must be non null.\n");
    uAssertMsg_v(NULL != return_queue_family_prop_count,
                 "[ vulkan ] Return count pointer must be non null.\n");

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, return_queue_family_prop_count, NULL);
    if (0 == *return_queue_family_prop_count)
    {
        uError("[ vulkan ] no device queue family properties were found for this device.\n");
        return false;
    }

    *return_queue_family_props = (VkQueueFamilyProperties*)calloc(*return_queue_family_prop_count,
                                                                  sizeof(VkQueueFamilyProperties));
    if (NULL == return_queue_family_props)
    {
        uError("[ vulkan ] Unable to allocate memory for queue family properties.\n");
        return false;
    }

    // Note: Does not return VkResult.
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                             return_queue_family_prop_count,
                                             *return_queue_family_props);

    return true;
}

bool
uvkGetQueueFamilyIndices(const VkPhysicalDevice physical_device,
                         const VkSurfaceKHR     surface,
                         u32* const restrict    return_graphics_queue_family_index,
                         u32* const restrict    return_present_queue_family_index)
{
    uAssertMsg_v(NULL != physical_device, "[ vulkan ] VkPhysicalDevice device must be valid.\n");
    uAssertMsg_v(NULL != surface, "[ vulkan ] VkSurfaceKHR must be valid.\n");
    uAssertMsg_v(NULL != return_graphics_queue_family_index,
                 "[ vulkan ] Return graphics family index ptr must be non null.\n");
    uAssertMsg_v(NULL != return_present_queue_family_index,
                 "[ vulkan ] Return present family index pointer must be non null.\n");

    // The total number of queues checked by this function.
    u32 num_queues = 0;

    typedef struct
    {
        u32  queue_family_index;
        bool validated;
    } QueueFamilyValid;

    VkQueueFamilyProperties* queue_family_props      = NULL;
    u32                      queue_family_prop_count = 0;
    if (false ==
        uvkGetQueueFamilyProperties(physical_device, &queue_family_props, &queue_family_prop_count))
    {
        uError("[ vulkan ] Unalbe to query queue family properties.\n");
        return false;
    }

    // Require graphics family.
    QueueFamilyValid graphics_family = { 0 };
    num_queues++;

    // Require present family.
    QueueFamilyValid present_family = { 0 };
    num_queues++;

    bool queues_satisfied = false;
    for (u32 queue_idx = 0; queue_idx < queue_family_prop_count; queue_idx++)
    {
        // Check grahpics capability for this family.
        if (queue_family_props[queue_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_family.queue_family_index = queue_idx;
            graphics_family.validated          = true;
        }

        // Check present capability for this family on the provided surface.
        VkBool32 present_capability = false;
        VkResult support_result     = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device,
                                                                       queue_idx,
                                                                       surface,
                                                                       &present_capability);
        if (VK_SUCCESS != support_result)
        {
            continue;
        }

        if (true == present_capability)
        {
            present_family.queue_family_index = queue_idx;
            present_family.validated          = true;
        }

        if (graphics_family.validated && present_family.validated)
        {
            *return_graphics_queue_family_index = graphics_family.queue_family_index;
            *return_present_queue_family_index  = present_family.queue_family_index;

            // Issue perf warning for different graphics/presentation queues.
            if (graphics_family.queue_family_index != present_family.queue_family_index)
            {
                uWarning("[ vulkan ][ perf ] Different graphics and prsent queue families.\n");
            }

            queues_satisfied = true;
            break;
        }
    }

    if (NULL != queue_family_props)
    {
        free(queue_family_props);
    }

    return queues_satisfied;
}

bool
uvkGetAllPhysicalDevices(VkInstance                        instance,
                         VkPhysicalDevice** const restrict return_physical_devices,
                         u32* const restrict               return_physical_device_count)
{
    VkResult success = vkEnumeratePhysicalDevices(instance, return_physical_device_count, NULL);
    if ((VK_SUCCESS != success) || (0 == *return_physical_device_count))
    {
        return false;
    }

    const size_t array_bytes = *return_physical_device_count * sizeof(VkPhysicalDevice);
    *return_physical_devices = (VkPhysicalDevice*)malloc(array_bytes);
    if (NULL == *return_physical_devices)
    {
        return false;
    }

    success = vkEnumeratePhysicalDevices(instance,
                                         return_physical_device_count,
                                         *return_physical_devices);
    if (VK_SUCCESS != success)
    {
        return false;
    }

    return true;
}

bool
uvkCreatePhysicalDevice(const VkInstance                         instance,
                        const VkSurfaceKHR                       surface,
                        const char** const restrict              user_device_extension_names,
                        const u16                                user_device_extension_name_count,
                        VkSurfaceCapabilitiesKHR* const restrict return_surface_capabilities,
                        VkSurfaceFormatKHR* const restrict       return_surface_format,
                        VkPresentModeKHR* const restrict         return_present_mode,
                        VkPhysicalDevice* const restrict         return_physical_device)
{
    if (user_device_extension_name_count)
    {
        uAssertMsg_v(user_device_extension_names && *user_device_extension_names,
                     "[ vulkan ] If the device extension quanitity is non zero, "
                     "the names ptr must be non null\n");
    }

    // [ cfarvin::TODO ] Correctness
    //
    // Can this fail? If so we should return bool from uvkGetAllPhysicalDevices and check for that here.
    VkPhysicalDevice* physical_devices      = NULL;
    u32               physical_device_count = 0;
    uvkGetAllPhysicalDevices(instance, &physical_devices, &physical_device_count);

    bool                device_chosen                   = false;
    VkPhysicalDevice    candidate_device                = NULL;
    VkSurfaceFormatKHR* potential_surface_formats       = NULL;
    VkPresentModeKHR*   potential_present_modes         = NULL;
    u32                 potential_surface_format_count  = 0;
    u32                 potential_present_mode_count    = 0;
    u32                 designated_surface_format_index = 0;
    u32                 designated_present_mode_index   = 0;
    u32                 designated_graphics_index       = 0;
    u32                 designated_present_index        = 0;
    size_t              validated_device_ext_count      = 0;
    for (u32 device_idx = 0; device_idx < physical_device_count; device_idx++)
    {
        validated_device_ext_count = 0;
        candidate_device           = physical_devices[device_idx];

        // Surface format info may have been allocated in the previous iteration.
        if (potential_surface_formats)
        {
            free(potential_surface_formats);
            potential_surface_formats = NULL;
        }

        // Present mode info may have been allocated in the previous iteration.
        if (potential_present_modes)
        {
            free(potential_present_modes);
            potential_present_modes = NULL;
        }

        // No need to continue checks for NULL device.
        if (NULL == candidate_device)
        {
            continue;
        }

        // Get Queue indicies.
        if (false == uvkGetQueueFamilyIndices(candidate_device,
                                              surface,
                                              &designated_graphics_index,
                                              &designated_present_index))
        {
            continue;
        }

        // Get surface capabilities.
        if (false ==
            uvkGetSurfaceCapabilities(candidate_device, surface, return_surface_capabilities))
        {
            continue;
        }

        // Query device extensions.
        if (false == uvkQueryExtensions(candidate_device,
                                        user_device_extension_names,
                                        user_device_extension_name_count,
                                        NULL,
                                        0,
                                        &validated_device_ext_count))
        {
            continue;
        }

        if (validated_device_ext_count != user_device_extension_name_count)
        {
            continue;
        }

        // Get surface formats.
        if (false == uvkGetSurfaceFormats(candidate_device,
                                          surface,
                                          &potential_surface_formats,
                                          &potential_surface_format_count))
        {
            continue;
        }

        // Get surface present modes.
        if (false == uvkGetPresentModes(candidate_device,
                                        surface,
                                        &potential_present_modes,
                                        &potential_present_mode_count))
        {
            continue;
        }

        // Get surface format index.
        if (false == uvkSelectSurfaceFormat(potential_surface_formats,
                                            potential_surface_format_count,
                                            VK_FORMAT_B8G8R8A8_SRGB,
                                            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                                            &designated_surface_format_index))
        {
            continue;
        }

        // Get present mode index.
        bool             suitable_present_mode_found = false;
        VkPresentModeKHR acceptable_present_modes[2] = {
            VK_PRESENT_MODE_MAILBOX_KHR, // Preferred.
            VK_PRESENT_MODE_FIFO_KHR     // Acceptable.
        };

        for (size_t present_mode_idx = 0; present_mode_idx < 2; present_mode_idx++)
        {
            if (true == uvkSelectPresentMode(potential_present_modes,
                                             potential_present_mode_count,
                                             acceptable_present_modes[present_mode_idx],
                                             &designated_present_mode_index))
            {
                suitable_present_mode_found = true;
                break;
            }
        }

        if (false == suitable_present_mode_found)
        {
            continue;
        }

        device_chosen = true;
        break;
    }

    if (false == device_chosen)
    {
        uError("[ vulkan ] Unable to find suitable device.\n");
        return false;
    }

    // Set the return parameters.
    *return_surface_format  = potential_surface_formats[designated_surface_format_index];
    *return_present_mode    = potential_present_modes[designated_present_mode_index];
    *return_physical_device = candidate_device;

    if (NULL == *return_physical_device)
    {
        uError("[ vulkan ] uvkCreatePhysicalDevice produced invalid results.\n");
        return false;
    }

    // If there's more than one physical device detected, report which one was chosen
    if (physical_device_count > 1)
    {
        VkPhysicalDeviceProperties physical_device_properties = { 0 };
        vkGetPhysicalDeviceProperties(candidate_device, &physical_device_properties);
        printf("[ vulkan ] Chosen device: %s\n", physical_device_properties.deviceName);
    }

    if (potential_surface_formats)
    {
        free(potential_surface_formats);
    }

    if (potential_present_modes)
    {
        free(potential_present_modes);
    }

    if (physical_devices)
    {
        free(physical_devices);
    }

    return true;
}

bool
uvkCreateDevices(const VkInstance                         instance,
                 const VkSurfaceKHR                       surface,
                 const char** const restrict              user_validation_layer_names,
                 const size_t                             user_validation_layer_name_count,
                 const char** const restrict              user_instance_extension_names,
                 const size_t                             user_instance_extension_name_count,
                 const char** const restrict              user_device_extension_names,
                 const size_t                             user_device_extension_name_count,
                 VkSurfaceCapabilitiesKHR* const restrict return_surface_capabilities,
                 VkSurfaceFormatKHR* const restrict       return_surface_format,
                 VkPresentModeKHR* const restrict         return_present_mode,
                 uvkActiveQueues* const restrict          return_active_queues,
                 VkDevice* const restrict                 return_logical_device,
                 VkPhysicalDevice* const restrict         return_physical_device)
{
    if (user_validation_layer_name_count)
    {
        uAssertMsg_v(user_validation_layer_names && *user_validation_layer_names,
                     "[ vulkan ] If the instance layer quanitity is non zero, the "
                     "names ptr must be non null.\n");
    }
    if (user_instance_extension_name_count)
    {
        uAssertMsg_v(user_instance_extension_names && *user_instance_extension_names,
                     "[ vulkan ] If the instance extension quanitity is non zero, "
                     "the names ptr must be non null.\n");
    }
    if (user_device_extension_name_count)
    {
        uAssertMsg_v(user_device_extension_names && *user_device_extension_names,
                     "[ vulkan ] If the device extension quanitity is non zero, "
                     "the names ptr must be non null.\n");
    }

    if (false == uvkCreatePhysicalDevice(instance,
                                         surface,
                                         user_device_extension_names,
                                         user_device_extension_name_count,
                                         return_surface_capabilities,
                                         return_surface_format,
                                         return_present_mode,
                                         return_physical_device))
    {
        uError("[ vulkan ] Unable to create physical device.\n");
        return false;
    }

    if (false == uvkCreateLogicalDevice(*return_physical_device,
                                        surface,
                                        user_validation_layer_names,
                                        user_validation_layer_name_count,
                                        user_device_extension_names,
                                        user_device_extension_name_count,
                                        return_logical_device,
                                        return_active_queues))
    {
        uError("[ vulkan ] Unable to create logical device.\n");
        return false;
    }

    return true;
}

void
uvkDestroyDevices(VkInstance                instance,
                  VkSurfaceKHR              surface,
                  VkDebugUtilsMessengerEXT* debug_messenger,
                  VkShaderModule*           shader_modules,
                  u32                       shader_module_count,
                  VkImage**                 images,
                  VkImageView**             image_views,
                  u32                       image_count,
                  VkSwapchainKHR            swapchain,
                  VkRenderPass              render_pass,
                  VkDevice                  logical_device)
{
    uAssertMsg_v(NULL != instance, "[ vulkan ] VkInstance must be non null.\n");
    uAssertMsg_v(NULL != surface, "[ vulkan ] VkSurfaceKHR must be non null.\n");
    uAssertMsg_v(NULL != logical_device, "[ vulkan ] VkDevice must be non null.\n");

    uvkDestroyShaderModules(logical_device, shader_modules, shader_module_count);
    uvkDestroyImageViews(logical_device, image_views, images, image_count);
    uvkDestroySwapchain(logical_device, swapchain);
    uvkDestroySurface(instance, surface);
    uvkDestroyRenderPass(logical_device, render_pass);

    if (NULL != debug_messenger)
    {
        uvkDestroyDebugMessenger(instance, *debug_messenger);
    }

    uvkDestroyLogicalDevice(logical_device);
    uvkDestroyInstance(instance);
}

bool
uvkCreateDebugMessenger(const VkInstance                         instance,
                        const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_ci,
                        VkDebugUtilsMessengerEXT* const restrict return_debug_messenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
      (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                "vkCreateDebugUtilsMessengerEXT");

    if (NULL == vkCreateDebugUtilsMessengerEXT)
    {
        uError("[ vulkan ] Function ptr for vkCreateDebugUtilsMessengerEXT was not acquired.\n");
        return false;
    }

    // [ cfarvin::TODO ] When createing vktriangle, make sure to pass out debug messenger function
    //                   as a member of the VkDebugUtilsMessengerEXT parameter.
    VkResult success = vkCreateDebugUtilsMessengerEXT(instance,
                                                      &debug_messenger_ci,
                                                      NULL,
                                                      return_debug_messenger);

    if (VK_SUCCESS != success)
    {
        uError("[ vulkan ] Failed to create debug messenger callback.\n");
        return false;
    }

    return true;
}

bool
uvkDestroyDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
{
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                 "vkDestroyDebugUtilsMessengerEXT");
    if (NULL == vkDestroyDebugUtilsMessengerEXT)
    {
        uError(
          "[ vulkan ] Unable to acquire function pointer for vkDestroyDebugUtilsMessengerEXT.\n");
        return false;
    }

    vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, NULL);
    return true;
}

bool
uvkQueryLayers(const char** const restrict user_layer_names, size_t* const user_layer_count)
{
    uAssertMsg_v(NULL != user_layer_names, "[ vulkan ] User layer name ptr must be non null.\n");
    uAssertMsg_v(0 != user_layer_count, "[ vulkan ] User layer count must be non zero.\n");

    u32                layer_prop_count = 0;
    VkLayerProperties* layer_props      = NULL;
    if (false == uvkGetInstanceLayers(&layer_props, &layer_prop_count))
    {
        return false;
    }

    size_t validated_layer_count = uvkValidateUserLayers(user_layer_names,
                                                         *user_layer_count,
                                                         layer_props,
                                                         layer_prop_count);
    if (NULL != layer_props)
    {
        free(layer_props);
    }

    *user_layer_count = validated_layer_count;
    return true;
}

// [ cfarvin::NOTE ] Correctness
//
// In previous versions, there was an "instance_create_info" structure which obtained the value
// of the count query from VkEnumerate(Device/Instance)ExtensionProperties. Now, we're storing this
// locally. In the calling code, we need to make sure that the data is still written to the instance
// create info or any other structure which may need the number of extensions. The same goes for the
// enabled extension names. This function _just_ queries for the names to see if they're on the
// system.
//
// Note: <>_device_extension_count will be __overwritten__ with the number of validated extension
// counts.
bool
uvkQueryExtensions(const VkPhysicalDevice      physical_device,
                   const char** const restrict user_device_extension_names,
                   size_t                      user_device_extension_name_count,
                   const char** const restrict user_instance_extension_names,
                   size_t                      user_instance_extension_name_count,
                   size_t* const restrict      matched_extension_count)
{
    // Query device extensions.
    if (0 != user_device_extension_name_count)
    {
        uAssert(NULL != user_device_extension_names);

        u32                    device_ext_count = 0;
        VkExtensionProperties* device_ext_props = NULL;
        if (false == uvkGetDeviceExtensions(physical_device, &device_ext_props, &device_ext_count))
        {
            uError("[ vulkan ] Unable to get device extensions.\n");
            return false;
        }

        *matched_extension_count = uvkValidateUserExtensions(user_device_extension_names,
                                                             user_device_extension_name_count,
                                                             device_ext_props,
                                                             device_ext_count);

        if (NULL != device_ext_props)
        {
            free(device_ext_props);
        }
    }

    // Query instance extensions.
    if (0 != user_instance_extension_name_count)
    {
        uAssert(NULL != user_instance_extension_names);

        u32                    instance_ext_count = 0;
        VkExtensionProperties* instance_ext_props = NULL;
        if (false == uvkGetInstanceExtensions(&instance_ext_props, &instance_ext_count))
        {
            uError("[ vulkan ] Unable to get instance extensions.\n");
            return false;
        }

        *matched_extension_count = uvkValidateUserExtensions(user_instance_extension_names,
                                                             user_instance_extension_name_count,
                                                             instance_ext_props,
                                                             instance_ext_count);

        if (NULL != instance_ext_props)
        {
            free(instance_ext_props);
        }
    }

    return true;
}

bool
uvkCreateInstance(const char* const restrict  application_name,
                  const char** const restrict user_validation_layer_names,
                  const size_t                user_validation_layer_name_count,
                  const char** const restrict user_instance_extension_names,
                  const size_t                user_instance_extension_name_count,
                  VkInstance*                 return_instance)
{
    if (user_validation_layer_name_count)
    {
        uAssertMsg_v(user_validation_layer_names && *user_validation_layer_names,
                     "[ vulkan ] If the instance layer quanitity is non zero, the "
                     "names ptr must be non null.\n");
    }

    if (user_instance_extension_name_count)
    {
        uAssertMsg_v(user_instance_extension_names && *user_instance_extension_names,
                     "[ vulkan ] If the instance extension quanitity is non zero, "
                     "the names ptr must be non null.\n");
    }

    size_t return_extension_count = 0;
    if (false == uvkQueryExtensions(NULL,
                                    NULL,
                                    0,
                                    user_instance_extension_names,
                                    user_instance_extension_name_count,
                                    &return_extension_count))
    {
        return false;
    }

    if (return_extension_count != user_instance_extension_name_count)
    {
        uError("[ vulkan ] Not all requried extensions are available on this device.\n");
        return false;
    }

    size_t return_layer_count = user_validation_layer_name_count;
    if (false == uvkQueryLayers(user_validation_layer_names, &return_layer_count))
    {
        uError("[ vulkan ] Unable to query layers.\n");
        return false;
    }

    if (return_layer_count != user_validation_layer_name_count)
    {
        uError("[ vulkan ] Not all requried layers are available on this device.\n");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT setup_debug_messenger_ci = default_debug_messenger_ci;
    setup_debug_messenger_ci.pfnUserCallback                    = uvkDebugMessenger;

    // [ cfarvin::TODO ] Feature
    //
    // Parameterize application version
    VkApplicationInfo application_info  = { 0 };
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName   = application_name;
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName        = uGetEngineName();
    application_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_ci    = { 0 };
    instance_ci.enabledLayerCount       = user_validation_layer_name_count;
    instance_ci.ppEnabledLayerNames     = user_validation_layer_names;
    instance_ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pApplicationInfo        = &application_info;
    instance_ci.pNext                   = &setup_debug_messenger_ci;
    instance_ci.enabledExtensionCount   = user_instance_extension_name_count;
    instance_ci.ppEnabledExtensionNames = user_instance_extension_names;

    VkResult success = vkCreateInstance(&instance_ci, NULL, return_instance);
    if (success != VK_SUCCESS)
    {
        uError("[ vulkan ] Unable to create vulkan instance.\n");
        return false;
    }

    return true;
}

void
uvkDestroyInstance(VkInstance instance)
{
    vkDestroyInstance(instance, NULL);
}

#if _WIN32
bool
uvkCreateWin32Surface(const VkInstance             instance,
                      HWND                         window_handle,
                      HINSTANCE                    h_instance,
                      VkSurfaceKHR* const restrict return_surface)
{
    uAssertMsg_v(instance, "[ vulkan ] VkInstance must be non zero.\n");
    uAssertMsg_v(return_surface, "[ vulkan ] VkSurfaceKHR ptr must non null.\n");
    uAssertMsg_v(!(*return_surface),
                 "[ vulkan ] VkSurfaceKHR must be null; will be overwritten.\n");

    VkWin32SurfaceCreateInfoKHR win32_surface_ci = default_win32_surface_ci;
    win32_surface_ci.hwnd                        = window_handle;
    win32_surface_ci.hinstance                   = h_instance;

    VkResult win32_surface_result = vkCreateWin32SurfaceKHR(instance,
                                                            &win32_surface_ci,
                                                            NULL,
                                                            return_surface);

    if (VK_SUCCESS != win32_surface_result)
    {
        uError("[ vulkan ] Unable to create win32 surface.\n");
        return false;
    }

    return true;
}
#endif // _WIN32

void
uvkDestroySurface(VkInstance instance, VkSurfaceKHR surface)
{
    vkDestroySurfaceKHR(instance, surface, NULL);
}

// [ cfarvin::REVISIT ] Needs to be re-written with new style.
// bool
// uDestroyVulkan()
// {
//     uVulkanInfo* v_info = (uVulkanInfo*)uGetVulkanInfo();
//     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n");

//     // Wait for device to be idle
//     if (v_info->logical_device)
//     {
//         vkDeviceWaitIdle(v_info->logical_device);
//     }

//     bool status = true;

//     // [ cfarvin::TODO ] Correctness
//     //
//     // All of these functions should return true/false to indicate success.
//     status &= uDestroyVulkanImageGroup();
//     status &= uDestroyVulkanSurfaceInfo();
//     status &= uDestroyVulkanCommandInfo();
//     status &= uDestroyVulkanQueueInfo();
//     status &= uDestroyVulkanRenderInfo();
//     status &= uDestroyVulkanInfo();

//     return status;
// }

// [ cfarvin::STUDY ]  Vulkan pipeline question.
//
// It would be nice to feel very confident on the function and management of the
// following:
//   VK_DEPENDENCY_BY_REGION_BIT,
//   VkPipelineVertexInputStateCreateInfo,
//   input_assembly_ci.primitiveRestartEnable,
//   multi_sample_ci.sampleShadingEnable,
//   VkPipelineColorBlendAttachmentState, color_blend_ci.logicOpEnable
