#if __linux__
#include <understone/nix_platform.h>
#elif _WIN32
#include <understone/win_platform.h>
#endif // _WIN32

#include "shaders/baked_shaders/vkTriangleShader.h"

#include <stdbool.h>
#include <understone/event_tools.h>
/* #include <understone/render_tools.h> */
#include <understone/shader_tools.h>
/* #include <understone/tests/tests.h> // __uTESTS_ENABLED__ = 0 to disable tests on startup */
#include <understone/vulkan_default_create_infos.h>
#include <understone/vulkan_tools.h>
#include <understone/vulkan_types.h>

/* static const size_t                    kVkTriangleNumShaderStages                               = 2; */
/* static VkPipelineShaderStageCreateInfo shader_stage_cis[kVkTriangleNumShaderStages] = { */
/*     0 */
/* }; */

/* static uVulkanShader      shaders[kVkTriangleNumShaderStages] = { vkTriangle_vert, */
/*                                                                          vkTriangle_frag }; */
/* static uVulkanRenderTools render_tools                        = { 0 }; */

static bool                  running           = true;
static uGameWindowProperties window_props      = { 0 };
static uSystemEvent          sys_event         = { 0 };
static uMousePos             mouse_coordinates = { 0 };
static u64                   input_state       = 0;
static uvkActiveQueues       active_queues     = { 0 };

#define SHADER_COUNT 2
static uShader* shaders[SHADER_COUNT] = { &vkTriangle_vert, &vkTriangle_frag };

static VkInstance               vk_instance             = NULL;
static VkDevice                 vk_logical_device       = NULL;
static VkPhysicalDevice         vk_physical_device      = NULL;
static VkDebugUtilsMessengerEXT vk_debug_messenger      = NULL;
static VkSurfaceKHR             vk_surface              = NULL;
static VkSurfaceCapabilitiesKHR vk_surface_capabilities = { 0 };
static VkSurfaceFormatKHR       vk_surface_format       = { 0 };
static VkPresentModeKHR         vk_present_mode         = { 0 };
static VkSwapchainKHR           vk_swapchain            = NULL;
static VkImage*                 vk_images               = NULL;
static VkImageView*             vk_image_views          = NULL;
static u32                      vk_image_count          = 0;
static VkRenderPass             vk_render_pass          = NULL;
static VkViewport               vk_viewport             = { 0 };
static VkPipelineLayout         vk_pipeline_layout      = { 0 };
static VkPipeline               vk_graphics_pipeline    = { 0 };

#if _WIN32
static POINT      mouse_point = { 0 };
static uWin32Info os_info     = { 0 };
#else
static uLinuxInfo os_info = { 0 };
#endif // _WIN32

LRESULT CALLBACK
uEngineWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            sys_event = uEventClose;
            break;
        }

        case WM_DESTROY:
        {
            sys_event = uEventClose;
            PostQuitMessage(0);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            uSetInputPressed(uMouse_left, &input_state);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            uSetInputPressed(uMouse_right, &input_state);
            break;
        }

        case WM_LBUTTONUP:
        {
            uSetInputReleased(uMouse_left, &input_state);
            break;
        }

        case WM_RBUTTONUP:
        {
            uSetInputReleased(uMouse_right, &input_state);
            break;
        }

        case WM_SIZE:
        {
            // [ cfarvin::TODO ] scaling/ortho
            window_props.width  = (u16)LOWORD(lParam);
            window_props.height = (u16)HIWORD(lParam);
            sys_event           = uEventResize;

            if (wParam == SIZE_MINIMIZED)
            {
                window_props.is_minimized = true;
            }
            else if (wParam == SIZE_RESTORED)
            {
                window_props.is_minimized = false;
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            GetCursorPos(&mouse_point);
            ScreenToClient(hwnd, &mouse_point);

            // Note: uMousePos has origin @ lower left == (0, 0, 0)
            mouse_coordinates.x = (u16)mouse_point.x;
            mouse_coordinates.y = (u16)(window_props.height - mouse_point.y);
            break;
        }
    }

    // Default Window Procedure
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void
VkTriangleDestroy()
{
    /* uDestroyRenderToolSyncMembers((uVulkanRenderTools*)&render_tools); */
    /* uDestroyShaderModules(&shaders[0], kVkTriangleNumShaderStages); */

    // [ cfarvin::TODO ] Destroy Win32Info.
    // [ cfarvin::TODO ] Correctness.
    //
    // This check should be performed in the future when the event loop is working again.
    /* uAssertMsg_v(!running, "[ engine ] Tear down called while ` kRunning == true`.\n"); */

    VkShaderModule shader_modules[SHADER_COUNT] = { shaders[0]->module, shaders[1]->module };
    uvkDestroyDevices(vk_instance,
                      vk_surface,
                      &vk_debug_messenger,
                      &shader_modules[0],
                      SHADER_COUNT,
                      &vk_images,
                      &vk_image_views,
                      vk_image_count,
                      vk_swapchain,
                      vk_render_pass,
                      vk_logical_device);

    uAssert(NULL == vk_images);
    uAssert(NULL == vk_image_views);
    /* printf("[ VkTriangleExample ] Graceful exit.\n"); */
}

bool
VkTriangleInit()
{
    u8          required_instance_extension_count   = 2;
    const char* kRequiredInstanceValidationLayers[] = { "VK_LAYER_KHRONOS_validation" };
    const char* kRequiredDeviceExtensions[]         = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const char* kRequiredInstanceExtensions[3]      = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                                   "VK_KHR_surface",
                                                   NULL };
#if _WIN32
    const char* const win_surface_extension = "VK_KHR_win32_surface";
    kRequiredInstanceExtensions[2]          = win_surface_extension;
    required_instance_extension_count++;
#endif // _WIN32

    // Instance creation.
    if (false == uvkCreateInstance("Vk Triangle Example",
                                   kRequiredInstanceValidationLayers,
                                   1,
                                   kRequiredInstanceExtensions,
                                   required_instance_extension_count,
                                   &vk_instance))
    {
        uvkDestroyInstance(vk_instance);
        uError("[ vk triangle ] Unable to create devices.\n");
        return false;
    }

    // Debug messenger creation.
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_ci = default_debug_messenger_ci;
    debug_messenger_ci.pfnUserCallback                    = uvkDebugMessenger;
    if (false == uvkCreateDebugMessenger(vk_instance, debug_messenger_ci, &vk_debug_messenger))
    {
        uvkDestroyDebugMessenger(vk_instance, vk_debug_messenger);
        uvkDestroyInstance(vk_instance);
        uError("[ vulkan ] Unable to create the debug messenger.\n");
        return false;
    }

#if _WIN32
    // Win32Info creation.
    if (false == uCreateWin32Info(&window_props, uEngineWindowProc, &os_info))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        uvkDestroyDebugMessenger(vk_instance, vk_debug_messenger);
        uvkDestroyInstance(vk_instance);
        uError("[ vk triangle ] Unable to create Win32Info.\n");
        return false;
    }

    // Surface creation.
    if (false == uvkCreateWin32Surface(vk_instance, os_info.window, os_info.instance, &vk_surface))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        uvkDestroySurface(vk_instance, vk_surface);
        uvkDestroyDebugMessenger(vk_instance, vk_debug_messenger);
        uvkDestroyInstance(vk_instance);
        uError("[ vk triangle ] Unable to create surface.\n");
        return false;
    }
#else
    // [ cfarvin::TODO ] Implement linux os_info & surface cration
    uFatal("Linux os_info & surface creation not implemented.");
#endif // _WIN32

    // Device creation.
    if (false == uvkCreateDevices(vk_instance,
                                  vk_surface,
                                  kRequiredInstanceValidationLayers,
                                  1,
                                  kRequiredInstanceExtensions,
                                  required_instance_extension_count,
                                  kRequiredDeviceExtensions,
                                  1,
                                  &vk_surface_capabilities,
                                  &vk_surface_format,
                                  &vk_present_mode,
                                  &active_queues,
                                  &vk_logical_device,
                                  &vk_physical_device))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create devices.\n");
        return false;
    }

    // Shader module creation.
    VkPipelineShaderStageCreateInfo shader_stage_cis[SHADER_COUNT] = { 0 };
    if (false ==
        uvkCreateShaders(vk_logical_device, SHADER_COUNT, &shader_stage_cis[0], &shaders[0]))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create shaders.\n");
        return false;
    }

    // Swapchain creation.
    u32 preferred_image_count = 0;
    if (false == uvkCreateSwapchain(vk_logical_device,
                                    vk_surface,
                                    vk_surface_capabilities,
                                    vk_surface_format,
                                    vk_present_mode,
                                    active_queues.graphics_queue_family_index,
                                    active_queues.present_queue_family_index,
                                    &window_props,
                                    &vk_swapchain,
                                    &preferred_image_count))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create swapchain.\n");
        return false;
    }

    // Viewport creation.
    VkExtent2D surface_extent = { 0 };
    uvkGetSurfaceExtent(vk_surface_capabilities, &window_props, &surface_extent);
    vk_viewport        = default_viewport;
    vk_viewport.width  = surface_extent.width;
    vk_viewport.height = surface_extent.height;

    // Swapchain image creation.
    if (false == uvkGetSwapchainImages(vk_logical_device,
                                       vk_swapchain,
                                       preferred_image_count,
                                       &vk_images,
                                       &vk_image_count))
    {
        // [ cfarvin::TODO ] Destroy Win32Info
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create VkImage(s).\n");
        return false;
    }

    // Image view creation.
    if (false == uvkCreateImageViews(vk_logical_device,
                                     vk_surface_format,
                                     vk_images,
                                     vk_image_count,
                                     0,
                                     VK_IMAGE_VIEW_TYPE_2D,
                                     default_component_mapping,
                                     default_sub_resource_range,
                                     &vk_image_views))
    {
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create VkImageView(s).\n");
        return false;
    }

    // Renderpass creation.
    VkAttachmentDescription attachment_description = default_attachment_description;
    attachment_description.format                  = vk_surface_format.format;

    VkSubpassDependency subpass_dependency = default_subpass_dependency;
    if (false == uvkCreateRenderPass(vk_logical_device,
                                     0, // Render pass create flags.
                                     0, // Subpass description flags.
                                     VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     &attachment_description,
                                     1,
                                     NULL,                          // Input attachments.
                                     0,                             // Input attachment count.
                                     &default_attachment_reference, // Color attachment
                                     NULL,                          // Resolve attachment
                                     1,                   // Color/Resolve attachment count.
                                     NULL,                // Preserve attachment.
                                     0,                   // Preserve attachment count.
                                     NULL,                // Depth stencil attachment (1).
                                     &subpass_dependency, // Subpass dependencies.
                                     1,                   // Subpass dependency count.
                                     &vk_render_pass))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        VkTriangleDestroy();
        uError("[ vk triangle ] Unable to create VkRenderPass.\n");
        return false;
    }

    // Graphics pipeline creation.
    VkRect2D frame_buffer_scissor = { 0 };
    frame_buffer_scissor.extent   = surface_extent;

    VkPipelineViewportStateCreateInfo viewport_state_ci = default_viewport_state_ci;
    viewport_state_ci.pViewports                        = &vk_viewport;
    viewport_state_ci.pScissors                         = &frame_buffer_scissor;

    VkPipelineDepthStencilStateCreateInfo* depth_stencil_disabled = NULL;
    VkPipelineDynamicStateCreateInfo       dynamic_state_ci       = default_dynamic_state_ci;
    const VkDynamicState                   dynamic_states[2]      = { VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_LINE_WIDTH };
    dynamic_state_ci.pDynamicStates                               = &dynamic_states[0];
    dynamic_state_ci.dynamicStateCount                            = 2;

    if (false == uvkCreateGraphicsPipeline(vk_logical_device,
                                           SHADER_COUNT,
                                           shader_stage_cis,
                                           default_vertex_input_state_ci,
                                           default_input_assembly_state_ci,
                                           viewport_state_ci,
                                           default_rasterization_state_ci,
                                           default_multisample_state_ci,
                                           *depth_stencil_disabled,
                                           default_color_blend_state_ci,
                                           dynamic_state_ci,
                                           default_pipeline_ci,
                                           vk_render_pass,
                                           0, // Subpass index
                                           &vk_pipeline_layout,
                                           &vk_graphics_pipeline))
    {
        // [ cfarvin::TODO ] Destroy Win32Info.
        VkTriangleDestroy();
        uError("[ vk triangle ] Could not create graphics pipeline.\n");
        return false;
    }

    //     uCreateVulkanGraphicsPipeline(kVkTriangleNumShaderStages, shader_stage_cis);
    //     uCreateVulkanFrameBuffers();
    //     uCreateVulkanCommandPool();
    //     uCreateVulkanCommandBuffers();
    //     uCreateRenderTools(&render_tools);

    return true;
}

void
VkTriangleHandleWindowResize()
{
    uDebugPrint("[ resize ] width: %d, height: %d\n", window_props.width, window_props.height);

    /* if (!window.is_minimized) */
    /* { */
    /*     uDestroyShaderModules(&shaders[0], kVkTriangleNumShaderStages); */
    /*     uCreateVulkanShaderModules(&shaders[0], */
    /*                                sizeof(shaders) / sizeof(uVulkanShader), */
    /*                                &shader_stage_cis[0]); */

    /*     const uVulkanInfo* const        v_info       = uGetVulkanInfo(); */
    /*     const uVulkanSurfaceInfo* const surface_info = uGetVulkanSurfaceInfo(); */
    /*     uRebuildVulkanSwapchain(&os_info, */
    /*                             &render_tools, */
    /*                             surface_info->surface, */
    /*                             v_info->physical_device, */
    /*                             v_info->logical_device, */
    /*                             v_info->instance, */
    /*                             shader_stage_cis, */
    /*                             kVkTriangleNumShaderStages, */
    /*                             &window); */

    /*     is_render_tool_outdated = false; */
    /* } */
}

void
VkTriangleRefreshInputState()
{
    uSystemEvent sys_event = uEventNone;

#if __linux__
    // [ cfarvin::RESTORE ] Re-implement
    // sys_event = uNixHandleEvents();
#elif _WIN32
    uWin32ProcessSystemEvents(&sys_event);
#else
    assert(0);
#endif // __linux__ _WIN32

    switch (sys_event)
    {
        case uEventNone:
        {
            break;
        }
        case uEventResize:
        {
            VkTriangleHandleWindowResize();
            break;
        }
        case uEventClose:
        {
            running = false;
            break;
        }
    }
}

void
VkTriangleGameLoop()
{
    /* while (running) */
    /* { */
    /*     if (!window.is_minimized) */
    /*     { */
    /*         uRenderFrame(&render_tools); */
    /*     } */

    /*     VkTriangleRefreshInputState(); */
    /* } */
}

int
main(int argc, char** argv)
{
    if (argc && argv)
    {
    }

    // [ cfarvin::TODO ] Tests should be their own project
    /* runAllTests(); */

    // [ cfarvin::TODO ] Move these to engine side; maybe a function
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

    if (false == VkTriangleInit())
    {
        uError("[ vk triangle ] Unable to initialize Vulkan.\n");
        return -1;
    }

    /* VkTriangleGameLoop(); */
    VkTriangleDestroy();

    puts("[ vk triangle ] Normal termination.");

    return 0;
}
