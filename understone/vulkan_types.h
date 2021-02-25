#ifndef __UE_VULKAN_TYPES_H__
#define __UE_VULKAN_TYPES_H__

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif // _WIN32

#include "debug_tools.h"
#include "type_tools.h"

#include <vulkan/vulkan.h>

/* typedef struct */
/* { */
/*     const VkCommandBuffer* command_buffers; */
/*     const VkCommandPool    command_pool; */
/*     const u32              num_command_buffers; */
/* } uVulkanCommandInfo; */
/* const uVulkanCommandInfo* uAPI_PRIME_VULKAN_COMMAND_INFO = NULL; */

/* const uVulkanCommandInfo* */
/* uGetVulkanCommandInfo() */
/* { */
/*     if (!uAPI_PRIME_VULKAN_COMMAND_INFO) */
/*     { */
/*         *(uVulkanCommandInfo**)&uAPI_PRIME_VULKAN_COMMAND_INFO = (uVulkanCommandInfo*)calloc( */
/*           1, */
/*           sizeof(uVulkanCommandInfo)); */
/*     } */

/*     return uAPI_PRIME_VULKAN_COMMAND_INFO; */
/* } */

/* typedef struct */
/* { */
/*     const VkSurfaceKHR             surface; */
/*     const VkSurfaceCapabilitiesKHR surface_capabilities; */
/*     const VkSurfaceFormatKHR       surface_format; */
/*     const VkPresentModeKHR         present_mode; */
/* } uVulkanSurfaceInfo; */
/* const uVulkanSurfaceInfo* uAPI_PRIME_VULKAN_SURFACE_INFO = NULL; */

/* const uVulkanSurfaceInfo* */
/* uGetVulkanSurfaceInfo() */
/* { */
/*     if (!uAPI_PRIME_VULKAN_SURFACE_INFO) */
/*     { */
/*         *(uVulkanSurfaceInfo**)&uAPI_PRIME_VULKAN_SURFACE_INFO = (uVulkanSurfaceInfo*)calloc( */
/*           1, */
/*           sizeof(uVulkanSurfaceInfo)); */
/*     } */

/*     return uAPI_PRIME_VULKAN_SURFACE_INFO; */
/* } */

/* // [ cfarvin::TODOD ] Correctness */
/* // */
/* // Removed num queue macro, graphics index macro, present index macro, */
/* // and changed the "queues" member to be a pointer rather than a statically */
/* // sized array via the num queues macro. Added a "queue_count" member to account */
/* // for this. We will need to: */
/* //   1) Make sure that these get populated properly when Vulkan is being set up. */
/* //   2) Make sure that this is being destroyed properly with a free on "queues". */
/* typedef struct */
/* { */
/*     const VkQueue* queues; */
/*     const u32      queue_count; */
/*     const u32      designated_graphics_index; */
/*     const u32      designated_present_index; */
/* } uVulkanQueueInfo; */
/* const uVulkanQueueInfo* uAPI_PRIME_VULKAN_QUEUE_INFO = NULL; */

/* const uVulkanQueueInfo* */
/* uGetVulkanQueueInfo() */
/* { */
/*     if (!uAPI_PRIME_VULKAN_QUEUE_INFO) */
/*     { */
/*         *(uVulkanQueueInfo**)&uAPI_PRIME_VULKAN_QUEUE_INFO = (uVulkanQueueInfo*)calloc( */
/*           1, */
/*           sizeof(uVulkanQueueInfo)); */
/*     } */

/*     return uAPI_PRIME_VULKAN_QUEUE_INFO; */
/* } */

/* typedef struct */
/* { */
/*     const VkInstance       instance; */
/*     const VkPhysicalDevice physical_device; */
/*     const VkDevice         logical_device; */
/* } uVulkanInfo; */
/* const uVulkanInfo* uAPI_PRIME_VULKAN_INFO = NULL; */

/* const uVulkanInfo* */
/* uGetVulkanInfo() */
/* { */
/*     if (!uAPI_PRIME_VULKAN_INFO) */
/*     { */
/*         *(uVulkanInfo**)&uAPI_PRIME_VULKAN_INFO = (uVulkanInfo*)calloc(1, sizeof(uVulkanInfo)); */
/*     } */

/*     return uAPI_PRIME_VULKAN_INFO; */
/* } */

/* typedef struct */
/* { */
/*     VkImage*             images; */
/*     VkImageView*         image_views;   // Note: Has num_images elements */
/*     VkFramebuffer*       frame_buffers; // Note: Has num_images elements */
/*     const u32            num_images; */
/*     const VkSwapchainKHR swapchain; */
/* } uVulkanImageGroup; */
/* const uVulkanImageGroup* uAPI_PRIME_VULKAN_IMAGE_GROUP = NULL; */

/* const uVulkanImageGroup* */
/* uGetVulkanImageGroup() */
/* { */
/*     if (!uAPI_PRIME_VULKAN_IMAGE_GROUP) */
/*     { */
/*         *(uVulkanImageGroup**)&uAPI_PRIME_VULKAN_IMAGE_GROUP = (uVulkanImageGroup*)calloc( */
/*           1, */
/*           sizeof(uVulkanImageGroup)); */
/*     } */

/*     return uAPI_PRIME_VULKAN_IMAGE_GROUP; */
/* } */

/* void */
/* uDestroyVulkanQueueInfo() */
/* { */
/*     uVulkanQueueInfo* queue_info = (uVulkanQueueInfo*)uGetVulkanQueueInfo(); */
/*     uAssertMsg_v(queue_info, "[ vulkan ] uVulkanQueueInfo ptr must be non null.\n"); */

/*     if (queue_info) */
/*     { */
/*         free(queue_info); */
/*     } */
/* } */

/* void */
/* uDestroyVulkanCommandBuffers() */
/* { */
/*     uVulkanInfo*        v_info       = (uVulkanInfo*)uGetVulkanInfo(); */
/*     uVulkanCommandInfo* command_info = (uVulkanCommandInfo*)uGetVulkanCommandInfo(); */

/*     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n"); */
/*     uAssertMsg_v(v_info->logical_device, "[ vulkan ] VkInstance must be non zero.\n"); */
/*     uAssertMsg_v(command_info, "[ vulkan ] uVulkanCommandInfo ptr must be non null.\n"); */

/*     if (command_info) */
/*     { */
/*         if (v_info) */
/*         { */
/*             vkFreeCommandBuffers(v_info->logical_device, */
/*                                  command_info->command_pool, */
/*                                  command_info->num_command_buffers, */
/*                                  command_info->command_buffers); */
/*         } */

/*         if (command_info->command_buffers) */
/*         { */
/*             free((void*)command_info->command_buffers); */
/*         } */
/*     } */
/* } */

/* void */
/* uDestroyVulkanCommandInfo() */
/* { */
/*     uVulkanInfo*        v_info       = (uVulkanInfo*)uGetVulkanInfo(); */
/*     uVulkanCommandInfo* command_info = (uVulkanCommandInfo*)uGetVulkanCommandInfo(); */

/*     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n"); */
/*     uAssertMsg_v(v_info->instance, "[ vulkan ] VkInstance must be non zero.\n"); */
/*     uAssertMsg_v(command_info, "[ vulkan ] uVulkanCommandInfo ptr must be non null.\n"); */

/*     if (command_info) */
/*     { */
/*         uDestroyVulkanCommandBuffers(); */

/*         if (v_info) */
/*         { */
/*             vkDestroyCommandPool(v_info->logical_device, command_info->command_pool, NULL); */
/*         } */
/*     } */

/*     if (command_info) */
/*     { */
/*         free(command_info); */
/*     } */
/* } */

/* void */
/* uDestroyVulkanSurfaceInfo() */
/* { */
/*     uVulkanInfo*        v_info       = (uVulkanInfo*)uGetVulkanInfo(); */
/*     uVulkanSurfaceInfo* surface_info = (uVulkanSurfaceInfo*)uGetVulkanSurfaceInfo(); */

/*     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n"); */
/*     uAssertMsg_v(v_info->instance, "[ vulkan ] VkInstance must be non zero.\n"); */
/*     uAssertMsg_v(v_info->logical_device, "[ vulkan ] VkInstance must be non zero.\n"); */
/*     uAssertMsg_v(surface_info, "[ vulkan ] uVulkanSurfaceInfo ptr must be non null.\n"); */

/*     if (surface_info && v_info->instance) */
/*     { */
/*         vkDestroySurfaceKHR(v_info->instance, surface_info->surface, NULL); */
/*     } */

/*     if (surface_info) */
/*     { */
/*         free(surface_info); */
/*     } */
/* } */

/* void */
/* uDestroyVulkanInfo() */
/* { */
/*     uVulkanInfo* v_info = (uVulkanInfo*)uGetVulkanInfo(); */

/*     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n"); */
/*     uAssertMsg_v(v_info->instance, "[ vulkan ] VkInstance must be non zero.\n"); */

/*     if (v_info) */
/*     { */
/*         if (v_info->instance) */
/*         { */
/* #if __UE_debug__ == 1 || __UE_vkForceValidation__ == 1 */
/*             // Destroy debug messenger */
/*             PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = */
/*               (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( */
/*                 v_info->instance, */
/*                 "vkDestroyDebugUtilsMessengerEXT"); */
/*             uAssertMsg_v(vkDestroyDebugUtilsMessengerEXT, */
/*                          "[ vulkan ] Unable to acquire fnptr: " */
/*                          "vkDestroyDebugUtilsMessengerEXT().\n"); */
/*             if (vkDestroyDebugUtilsMessengerEXT && vulkan_main_debug_messenger) */
/*             { */
/*                 vkDestroyDebugUtilsMessengerEXT(v_info->instance, */
/*                                                 vulkan_main_debug_messenger, */
/*                                                 NULL); */
/*             } */
/* #endif // __UE_debug__ == 1 || __UE_vkForceValidation__ == 1 */
/*         } */

/*         if (v_info->logical_device) */
/*         { */
/*             vkDestroyDevice(v_info->logical_device, NULL); */
/*         } */

/*         if (v_info->instance) */
/*         { */
/*             vkDestroyInstance(v_info->instance, NULL); */
/*         } */
/*     } */

/*     if (v_info) */
/*     { */
/*         free(v_info); */
/*     } */
/* } */

/* void */
/* uDestroyVulkanImageGroup() */
/* { */
/*     uVulkanInfo*       v_info      = (uVulkanInfo*)uGetVulkanInfo(); */
/*     uVulkanImageGroup* image_group = (uVulkanImageGroup*)uGetVulkanImageGroup(); */

/*     uAssertMsg_v(v_info, "[ vulkan ] uVulkanInfo ptr must be non null.\n"); */
/*     uAssertMsg_v(v_info->logical_device, "[ vulkan ] VkDevice ptr must be non null.\n"); */
/*     uAssertMsg_v(image_group, "[ vulkan ] uVulkanImageGroup ptr must be non null.\n"); */

/*     uDebugPrint("Destroying image group components...\n"); */
/*     if (image_group) */
/*     { */
/*         if (v_info && v_info->logical_device) */
/*         { */
/*             // Destroy image views, frame buffers */
/*             for (u32 image_idx = 0; image_idx < image_group->num_images; image_idx++) */
/*             { */
/*                 if (image_group->image_views) */
/*                 { */
/*                     vkDestroyImageView(v_info->logical_device, */
/*                                        image_group->image_views[image_idx], */
/*                                        NULL); */
/*                 } */

/*                 if (image_group->frame_buffers) */
/*                 { */
/*                     vkDestroyFramebuffer(v_info->logical_device, */
/*                                          image_group->frame_buffers[image_idx], */
/*                                          NULL); */
/*                 } */
/*             } */
/*         } */

/*         if (image_group->swapchain) */
/*         { */
/*             uDebugPrint("Destroying old swap chain...\n"); */
/*             vkDestroySwapchainKHR(v_info->logical_device, image_group->swapchain, NULL); */
/*         } */

/*         if (image_group->images) */
/*         { */
/*             free(image_group->images); */
/*         } */

/*         if (image_group->image_views) */
/*         { */
/*             free(image_group->image_views); */
/*         } */

/*         if (image_group->frame_buffers) */
/*         { */
/*             free(image_group->frame_buffers); */
/*         } */
/*     } */

/*     if (image_group) */
/*     { */
/*         free(image_group); */
/*     } */
/* } */

#endif // __UE_VULKAN_TYPES_H__
