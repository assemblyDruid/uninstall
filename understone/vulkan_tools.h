/*
 * Guiding Principals
 *
 * Vulkan is a verbose API. Thus, this library is also very verbose in it's calls to
 * create and manage Vulkan types and objects. The guiding idea is that each function
 * call is as verbose as necessary to create Vulkan objects with all options provided
 * by vulkan made available to the user, except where other parameters in a function
 * call can be used to deduce some members of a Vulkan structure or task. This is
 * particularly relevant to the "Create" functions. Though this means more verbocity
 * in calling and using functions in this library, doing so  directly lends an accurate
 * view of all of the information needed by Vulkan to perform tasks and create objects.
 * A corresponding principal is that the user is free to build their own functions which
 * hedge this verbocity for specific uses.
 */

#include <stdbool.h>

#ifndef __UE_VULKAN_TOOLS_H__
#define __UE_VULKAN_TOOLS_H__

#if __linux__
#include <understone/nix_platform.h>
#elif _WIN32
#include <understone/win_platform.h>
#endif // _WIN32

#include "type_tools.h"
#include "vulkan/vulkan.h"

/// A grouping of active queues.
typedef struct
{
    VkQueue graphics_queue;
    VkQueue present_queue;
    u32     graphics_queue_family_index;
    u32     present_queue_family_index;
    u32     active_queue_family_count;
} uvkActiveQueues;

// [ cfarvin::TODO ] Semantics
//
// Rename this function to uvkDestroy or uvkTearDown.
/*
 * @brief Destroy all vulkan objects.
 */
bool
uDestroyVulkan();

/*
 * @brief Create a Win32 surface.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] window_handle A valid Win32 HWND.
 * @param[in] h_istance A valid Win32 HISNTACE.
 * @param[out] return_surface The surface to be created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateWin32Surface(const VkInstance instance,
                      HWND             window_handle,
                      HINSTANCE        h_instance,
                      VkSurfaceKHR*    return_surface);

/*
 * @brief Destroys a valid VkSurfaceKHR.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] surface A valid VkSurfaceKHR to destroy.
 */
void
uvkDestroySurface(VkInstance instance, VkSurfaceKHR surface);

/*
 * @brief Create and validate a vulkan instance.
 *
 * @param[in] application_name The name of the application.
 * @param[in] user_validation_layer_names An array of instance layers to query and enable.
 * @param[in] user_validation_layer_name_count The number of validation layers.
 * @param[in] user_instance_extension_names An array of instance extensions to query and enable.
 * @param[in] user_instance_extension_name_count The number of instance layers.
 * @param[out] return_vulkan_instance The Vulkan instance created by this function.
 */
bool
uvkCreateInstance(const char* const restrict  application_name,
                  const char** const restrict user_validation_layer_names,
                  const size_t                user_validation_layer_name_count,
                  const char** const restrict user_instance_extension_names,
                  const size_t                user_instance_extension_name_count,
                  VkInstance*                 return_instance);

/*
 * @brief Destroy a VkInstance.
 *
 * @param[in] instance A valid VkInstance.
 */
void
uvkDestroyInstance(VkInstance instance);

/*
 * @brief Queries the current Vulkan context for the provided extension names.
 *
 * When querying for device extensions only, pass NULL for all instance extension parameters.
 * When querying for instance extensions only, pass NULL for the first three parameters.
 *
 * @param[in] physical_device The physical device associated with the application.
 *            Can be NULL when querying for instance extensions only.
 * @param[in] user_device_extension_names A pointer to an array of instanceextension names to query
 *            for. Can be NULL when queyring for instance extensions only.
 * @param[in] user_device_extension_count The number of elements in the device extension name
 *                array.
 * @param[in] user_instance_extension_names A pointer to an array of instance extensions to query
 *            for. Can be NULL if querying for device extensions only.
 * @param[in] user_instance_extension_count The number of elements in the instance extension
 *                name array.
 * @param[out] matched_extension_count The number of extensions from the user extension array that
 *             matched available extensions.
 *
 * @return True on success, false otherwise.
 */
bool
uvkQueryExtensions(const VkPhysicalDevice      physical_device,
                   const char** const restrict user_device_extension_names,
                   size_t                      user_device_extension_count,
                   const char** const restrict user_instance_extension_names,
                   size_t                      user_instance_extension_count,
                   size_t* const restrict      matched_extension_count);

/*
 * @brief Queries the current Vulkan context for the provided layer names.
 *
 * Note: The layer count parameter initially indicates the length of the layer name array parameter.
 * However, it will be _overwritten_ on return to indicate the number of matches between available
 * layers and user requested layers. Callers should store the original value and compare post-return.
 * Callers should first see the uvkCreateDevices and uvkCreateIstance functions, which incorporate this
 * functionality.
 *
 * @param[in] user_layer_names A pointer to an array of layer names to user would like to enable.
 * @param[in,out] user_layer_count A pointer to the length of the name array parameter. On return,
 *                the number of available layers requested will be stored at this address.
 */
bool
uvkQueryLayers(const char** const restrict user_layer_names, size_t* const user_layer_count);

/*
 * @brief The default debug messenger callback. See Vulkan documentation for parmeter details.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL
uvkDebugMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity_bits,
                  VkDebugUtilsMessageTypeFlagsEXT             message_type_bits,
                  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                  void*                                       user_data);

/*
 * @brief Create a debug callback messenger. Callers should first see the uvkCreateDevices function,
 *        which incorporates this functionality.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] debug_messenger_ci If non NULL, will be used to create the debug messenger. Otherwise
 *                               the default values will be used.
 * @param[out] return_debug_messenger The debug messenger to be created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateDebugMessenger(const VkInstance                         instance,
                        const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_ci,
                        VkDebugUtilsMessengerEXT* const restrict return_debug_messenger);

/*
 * @brief Destroys a VkDebugUtilsMessengerEXT.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] debug_messenger A valid VkDebugUtilsMessengerEXT to destroy.
 *
 * @return True on success, false otherwise.
 */
bool
uvkDestroyDebugMessenger(const VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger);

/*
 * @brief Creates a logical and physical device, along with the objects that must be compatible
 *        with logical and physical devices. Callers should first see the uvkCreateInstance function,
 *        which incorporates this functionality.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[in] user_validation_layer_names A pointer to an array of validation layer names to enable.
 * @param[in] user_validation_layer_name_count The number of elements in the validation layer name array.
 * @param[in] user_instance_extension_names A pointer to an array of instance extension names to enable.
 * @param[in] user_instance_extension_name_count The number of elements in the instance extension name array.
 * @param[in] user_device_extension_names A pointer to an array of device extension names to enable.
 * @param[in] user_device_extension_name_count The number of elements in the device extension name array.
 * @param[out] return_surface_capabilities The VkSurfaceCapabilitiesKHR structure created by this function.
 * @param[out] return_surface_format The VkSurfaceFormatKHR structure created by this function.
 * @param[out] return_present_mode The VkPresentModeKHR structure created by this function.
 * @param[out] return_active_queues The VkActiveQueues structure created by this function.
 * @param[out] return_logical_device The VkDevice created by this function.
 * @param[out] return_physical_device The VkPhysicalDevice created by this function.
 *
 * @return True on success, false otherwise.
 */
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
                 VkPhysicalDevice* const restrict         return_physical_device);

/*
 * @brief Destroys a VkDevice and all objects which rely on it.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[in] debug_messenger If not NULL, a valid VkDebugUtilsMessengerEXT.
 * @param[in] shader_modules An array of valid VkShaderModule.
 * @param[in] shader_module_count The number of elements in the VkShaderModule array.
 * @param[in] images A pointer to an array of valid VkImage(s).
 * @param[in] image_views A pointer to an array of valid VkImageView(s).
 * @param[in] image_count The number of elements in the VkImageView array.
 * @param[in] render_pass A valid VkRenderPass.
 * @param[in] swapchain A valid VkSwapchainKHR.
 * @param[in] logical_device A valid VkDevice.
 */
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
                  VkDevice                  logical_device);

/*
 * @brief Creates a physical device, and the structures that must be compatible with it. Callers
 *        should first see the uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] instance A valid VkInstance.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[in] user_device_extension_names A pointer to an array of device extension names to enable.
 * @param[in] user_device_extension_name_count The number of elements in the device extension name array.
 * @param[out] return_surface_capabilities The VkSurfaceCapabilitiesKHR structure created by this function.
 * @param[out] return_surface_format The VkSurfaceFormatKHR structure created by this function.
 * @param[out] return_present_mode The VkPresentModeKHR structure created by this function.
 * @param[out] return_physical_device The VkPhysicalDevice created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreatePhysicalDevice(const VkInstance                instance,
                        const VkSurfaceKHR              surface,
                        const char** const restrict     user_device_extension_names,
                        const u16                       user_device_extension_count,
                        VkSurfaceCapabilitiesKHR* const return_surface_capabilities,
                        VkSurfaceFormatKHR* const       return_surface_format,
                        VkPresentModeKHR* const         return_present_mode,
                        VkPhysicalDevice* const         return_physical_device);

/*
 * @brief Obtain all the physical devices on this system. Callers should first see the uvkCreateDevices
 *        function, which incorporates this functionality.
 *
 * @param[in] instance A valid VkInstance.
 * @param[out] An array of physical devices.
 * @param[out] The number of elements in the physical devices array.
 *
 * @return True on succes, false otherwise.
 */
bool
uvkGetAllPhysicalDevices(VkInstance                        instance,
                         VkPhysicalDevice** const restrict return_physical_devices,
                         u32* const restrict               return_physical_device_count);

/*
 * @brief Obtains the graphics and present queue index for the given surface and physical device.
 *        Callers should first see the uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] pysical_device A valid VkPhysicalDevice.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[out] return_graphics_queue_family_index On successful return, the index of the graphics queue.
 * @param[out] return_present_queue_family_index On successful return, the index of the present queue.
 *
 * @return True on succes, false otherwise..
 */
bool
uvkGetQueueFamilyIndices(const VkPhysicalDevice physical_device,
                         const VkSurfaceKHR     surface,
                         u32* const restrict    return_graphics_queue_family_index,
                         u32* const restrict    return_present_queue_family_index);

/*
 * @brief Obtain all queue family properties for the given physical device. Callers should first see
 *        the uvkGetQueueIndices function, which incoporates this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[out] return_queue_family_props An array of VkQueueFamilyProperties to populate.
 * @param[out] return_queue_family_porp_count The number of elements returned in the queue family
 *             properties array.
 *
 * @return True on success, false otherwise.
 */
bool
uvkGetQueueFamilyProperties(const VkPhysicalDevice                   physical_device,
                            VkQueueFamilyProperties** const restrict return_queue_family_props,
                            u32* const return_queue_family_prop_count);

/*
 * @brief Counts the number of unique queue family indicies.
 *
 * @param[in] graphics_queue_family_index The index of the graphics queue family.
 * @param[in] present_queue_family_index The index of the present queue family.
 * @param[out] return_unique_queue_family_count The number of uniqueue queue family indices from all
 *             input parameters.
 */
void
uvkCountUniqueQueueFamilies(const u32     graphics_queue_family_index,
                            const u32     present_queue_family_index,
                            u32* restrict return_unique_queue_family_count);

/*
 * @brief Creates VkDeviceQueueCreateInfo(s). This function allccates memeory not freed in this scope.
 *        Callers should first see the uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] graphics_queue_family_index The index of the graphics queue family.
 * @param[in] present_queue_family_index The index of the present queue family.
 * @param[in] device_queue_ci A VkDeviceQueueCreateInfo to use as a template.
 * @param[out] return_device_queue_cis A pointer to an array of VkDeviceQueueCreateInfo(s) to be created.
 * @param[out] return_uniqueue_queue_count A pointer to store the quantity of unique queues discovered.
 *
 * @return True on success, false otherise.
 */
bool
uvkGetDeviceQueueCreateInfos(const u32                                graphics_queue_family_index,
                             const u32                                present_queue_family_index,
                             const VkDeviceQueueCreateInfo            device_queue_ci,
                             VkDeviceQueueCreateInfo** const restrict return_device_queue_cis,
                             u32* const restrict                      return_unique_queue_count);

/*
 * @brief Obtain the surface capabilities for a given VkSurfaceKHR. Callers should first see the
 *        uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[out] return_surface_capabilities The surface capabilities pertaining to the VkSurfaceKHR parameter.
 *
 * @return True on success, false otherwise.
 */
bool
uvkGetSurfaceCapabilities(const VkPhysicalDevice                   physical_device,
                          const VkSurfaceKHR                       surface,
                          VkSurfaceCapabilitiesKHR* const restrict return_surface_capabilities);

/*
 * @brief Obtain all supported surface formats for a given VkSurfaceKHR. Callers should first see the
 *        uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[out] return_surface_formats An array of the supported surface formats for the surface parameter.
 * @param[out] return_surface_format_count The number of elements in the surface format array.
 *
 * @return True on success, false otherwise.
 */
bool
uvkGetSurfaceFormats(const VkPhysicalDevice              physical_device,
                     const VkSurfaceKHR                  surface,
                     VkSurfaceFormatKHR** const restrict return_surface_formats,
                     u32* const restrict                 return_surface_format_count);

/*
 * @brief Obtain all supported present modes for a given VkSurfaceKHR. Callers should first see the
 *        uvkCreateDevices function, which incorporates this functionality. Does not log error
 *        messages as it is typically called within a verification loop where invalid options are
 *        expected on some loop iterations. Callers should first see the uvkCreateDevices function,
 *        which incorporates this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[in] surface A valid VkSurface.
 * @param[out] return_present_modes An array of prsent modes for the given VkSurfaceKHR.
 * @param[out] return_present_mode_count The number of elements in the present mode array.
 *
 * @return True on success, false otherwise.
 */
bool
uvkGetPresentModes(const VkPhysicalDevice            physical_device,
                   const VkSurfaceKHR                surface,
                   VkPresentModeKHR** const restrict return_present_modes,
                   u32* const restrict               return_present_mode_count);

/*
 * @brief Selects a surface format based on the preferred VkFormat and VkColorSpaceKHR paramters,
 *        if possible. Callers should first see the uvkCreateDevices function, which incorporates
 *        this functionality.
 *
 * @param[in] surface_formats An array of all surface formats to be considered.
 * @param[in] surface_format_count The number of elements in the surface format array.
 * @param[in] desired_format The preferred VkFormat.
 * @param[in] desired_color_sapce The preferred VkColorSpaceKHR.
 * @param[out] return_surface_format The index in the surface format array that meets the prameter
 *             requirements, if any.
 *
 * @return True on success, false otherwise.
 */
bool
uvkSelectSurfaceFormat(const VkSurfaceFormatKHR* const restrict surface_formats,
                       const u32                                surface_format_count,
                       const VkFormat                           desired_format,
                       const VkColorSpaceKHR                    desired_color_space,
                       u32* const restrict                      return_surface_format_index);

/*
 * @brief Selects a present mode based based on the preffered present mode, if possible. Callers
 *        should first see the uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] present_modes An array of present modes to consider.
 * @param[in] present_mode_count The number of elements in the present mode array.
 * @param[in] desired_present_mode The preferred present mode.
 * @param[out] return_present_mode_index The index in the present mode array that conforms to the
 *             preferred parameters, if possible.
 *
 * @return True on success, false otherwise.
 */
bool
uvkSelectPresentMode(const VkPresentModeKHR* const restrict present_modes,
                     const u32                              present_mode_count,
                     VkPresentModeKHR                       desired_present_mode,
                     u32* const restrict                    return_present_mode_index);

/*
 * @brief Creates a logical device based on the logical_device_ci_template parameter. The extension
 *        and layer names and counts for the VkDeviceCreateInfo will be handled by this function based
 *        on other parameters. This function also returns a uvkActiveQueues structure for the created
 *        logical device. Callers should first see the uvkCreateDevices function, which incorporates
 *        this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[in] surface A valid VkSrufaceKHR.
 * @param[in] user_validation_layer_names A pointer to an array of validation layer names.
 * @param[in] user_validation_layer_name_count The number of elements in the layer names parameter.
 * @param[in] user_device_extension_names A pointer to an array of device extensions to enable.
 * @param[in] user_device_extension_name_count The number of elements in the device extension names
 *            parameter.
 * @param[out] return_logical_device A pointer to the logical device that will be created.
 * @param[out] return_active_queues The active present and graphcis queues associated with this
 *             logical device.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateLogicalDevice(const VkPhysicalDevice          physical_device,
                       const VkSurfaceKHR              surface,
                       const char** const restrict     user_validation_layer_names,
                       const size_t                    user_validation_layer_name_count,
                       const char** const restrict     user_device_extension_names,
                       const size_t                    user_device_extension_name_count,
                       VkDevice*                       return_logical_device,
                       uvkActiveQueues* const restrict return_active_queues);

/*
 * @brief Destroys a valid VkDevice.
 *
 * @param[in] logical_device A valid VkDevice.
 */
void
uvkDestroyLogicalDevice(VkDevice logical_device);

/*
 * @brief Validates that the requested layers are available on the system. Callers should first see the
 *        uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] user_layers A pointer to an array of layer names.
 * @param[in] user_layer_count The number of elements in the user layer name parameter.
 * @param[in] layer_properties An array of VkLayerProperties obtained.
 * @param[in] layer_property_count The number of elements in the layer properties parameter.
 *
 * @return The number of layers from the user layer array that are also available in the
 *         VkLayerProperties array.
 */
size_t
uvkValidateUserLayers(const char** const restrict             user_layers,
                      const size_t                            user_layer_count,
                      const VkLayerProperties* const restrict layer_properties,
                      const size_t                            layer_property_count);

/*
 * @brief Validates that the requested extensions are available on the system. Callers should first
 *        look at the uvkQueryExtensions function, which incorporates this functionality.
 *
 * @param[in] user_extensions An array of extension names.
 * @param[in] user_extension_count The number of elements in the extension name array.
 * @param[in] extension_properties An array of VkExtensionProperties.
 * @param[in] extension_properties_count The number of elements in the extension properties array.
 *
 * @return The number of extensions from the user extension array that are also available in the
 *         VkExtensionProperties array.
 */
size_t
uvkValidateUserExtensions(const char** const restrict                 user_extensions,
                          const size_t                                user_extension_count,
                          const VkExtensionProperties* const restrict extension_properties,
                          const size_t                                extension_property_count);

/*
 * @brief Obtain all available VkLayerProperties available on this system. Callers should first look
 *        at the uVkQueryLayers function, which incorporates this functionality.
 *
 * @param[out] return_layer_properties A pointer to an array of VkLayerProperties to be populated.
 * @param[out] return_layer_count The number of elements in the returned layer property array.
 */
bool
uvkGetInstanceLayers(VkLayerProperties** const restrict return_layer_properties,
                     u32* const                         return_layer_count);

/*
 * @brief Obtain all available VkExtensionProperties for VkInstance(s) available on this system.
 *        Callers should first look at the uvkQueryExtensions function, which incorporates
 *        this functionality.
 *
 * @param[out] return_extension_properties A pointer to an array of VkExtensionProperties for
 *             VkInstance(s) to be populated.
 * @param[out] return_extension_count The number of elements in the returned extension property
 *             array.
 */
bool
uvkGetInstanceExtensions(VkExtensionProperties** const restrict return_extension_properties,
                         u32* const                             return_extension_count);

/*
 * @brief Obtain all available VkExtensionProperties for VkDevice(s) available on this system.
 *        Callers should first look at the uvkQueryExtensions function, which incorporates
 *        this functionality.
 *
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[out] return_extension_properties A pointer to an array of VkExtensionProperties for
 *             VkDevice(s) to be populated.
 * @param[out] return_extension_count The number of elements in the returned extension property
 *             array.
 */
bool
uvkGetDeviceExtensions(const VkPhysicalDevice                 physical_device,
                       VkExtensionProperties** const restrict return_extension_properties,
                       u32* const                             return_extension_count);

/*
 * @brief Obtains the surface extent for a given VkSurfaceCapabilitiesKHR and game window. Callers
 *        should first see the uvkCreateDevices function, which incorporates this functionality.
 *
 * @param[in] surface_capabilities A valid VkSurfaceCapabilitiesKHR.
 * @param[in] game_window_props A valid uGameWindowProperties.
 * @param[out] surface_extent The obtained surface extent.
 */
void
uvkGetSurfaceExtent(const VkSurfaceCapabilitiesKHR              surface_capabilities,
                    const uGameWindowProperties* restrict const game_window_props,
                    VkExtent2D* const restrict                  surface_extent);

/*
 * @brief Creates a swapchain and all of the objects that must conform with it.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] surface A valid VkSurfaceKHR.
 * @param[in] surface_capabilities A valid VkSurfaceCapabilitiesKHR pertaining to the surface
 *            parameter.
 * @param[in] surface_format A valid VkSurfaceFormatKHr pertaining to the surface parameter.
 * @param[in] present_mode A valid VkPresentModeKHR pertaining to the surface parameter.
 * @param[in] graphics_queue_family_index The index corresponding to the graphics queue family.
 * @param[in] present_queue_family_index The index corresponding to the present queue family.
 * @param[in] game_window_props A valid uGameWindowProperties.
 * @param[out] return_swapchain The swapchain to be created.
 * @param[out] return_preferred_image_count The preferred number of VkImage(s) to be created
 *             for the swapchain.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateSwapchain(const VkDevice                              logical_device,
                   const VkSurfaceKHR                          surface,
                   const VkSurfaceCapabilitiesKHR              surface_capabilities,
                   const VkSurfaceFormatKHR                    surface_format,
                   const VkPresentModeKHR                      present_mode,
                   const u32                                   graphics_queue_family_index,
                   const u32                                   present_queue_family_index,
                   const uGameWindowProperties* const restrict game_window_props,
                   VkSwapchainKHR*                             return_swapchain,
                   u32* const restrict                         return_preferred_image_count);

/*
 * @brief Destroys a previously created swapchain.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] swapchain A valid VkSwapchainKHR to be destroyed.
 */
void
uvkDestroySwapchain(const VkDevice logical_device, VkSwapchainKHR swapchain);

/*
 * @brief Creates and allocates VkImages for a given VkSwapchainKHR.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] swapchain A valid VkSwapchainKHR.
 * @param[in] requested_image_count The preferred number of images for the swapchain.
 * @param[in] return_images An array to store the VkImage(s) created by this function.
 * @param[in] return_image_count The number of VkImage(s) created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkGetSwapchainImages(const VkDevice       logical_device,
                      const VkSwapchainKHR swapchain,
                      u32                  requested_image_count,
                      VkImage**            return_images,
                      u32*                 return_image_count);

/*
 * @brief Creates VkImageView(s).
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] surface_format A valid SurfaceFormatKHR.
 * @param[in] images An array of valid VkImage(s).
 * @param[in] image_count The number of elements in the VkImage array.
 * @param[in] flags A valid VkImageViewCreateFlags.
 * @param[in] view_type A valid VkImageViewType.
 * @param[in] component_mapping A valid VkComponentMapping.
 * @param[in] sub_resource_range A valid VkImageSubresourceRange.
 * @param[out] return_image_views The VkImageView(s) created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateImageViews(const VkDevice                logical_device,
                    const VkSurfaceFormatKHR      surface_format,
                    const VkImage* const restrict images,
                    const u32                     image_count,
                    const VkImageViewCreateFlags  flags,
                    const VkImageViewType         view_type,
                    const VkComponentMapping      component_mapping,
                    const VkImageSubresourceRange sub_resource_range,
                    VkImageView** restrict        return_image_views);

/*
 * @brief Destroyes VkImageView(s).
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] image_views A pointer to an array of VkImageView(s) to destroy.
 * @param[in] images A pointer to an array of VkImage(s) to destroy.
 * @param[in] image_view_count The nunber of elements int he VkImageView array.
 */
void
uvkDestroyImageViews(const VkDevice logical_device,
                     VkImageView**  image_views,
                     VkImage**      images,
                     const u32      image_view_count);

/*
 * @brief Creates a graphics command pool.
 *
 * @param[in] logical_device A valid logical device.
 * @param[in] graphics_queue_family_index The index of the graphics queue family on the provided
 *            logical device.
 * @param[in] flags A valid VkCommandPoolCreateFlags.
 * @param[out] return_command_pool The command pool created by this function.
 */
bool
uvkCreateGraphicsCommandPool(const VkDevice           logical_device,
                             const u32                graphics_queue_family_index,
                             VkCommandPoolCreateFlags flags,
                             VkCommandPool*           return_command_pool);

/*
 * @brief Destroys a valid VkCommandPool.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] command_ool A valid VkCommandPool to destroy.
 */
void
uvkDestroyCommandPool(const VkDevice logical_device, VkCommandPool command_pool);

/*
 * @brief Rebuilds a swapcahin. It is considered undefined behavior to call this method when the
 *        game window is minimized.
 *
 * @param[in] os_info A valid os_info. May need to be type-casted to suit the OS you're calling from.
 * @param[in] physical_device A valid VkPhysicalDevice.
 * @param[in] logical_device A valid VkDevice.
 * @param[in] instance A valid VkInstance.
 * @param[in] game_window_props A valid uGameWindowProperties.
 * @param[in] graphics_queue_family_index The graphics queue family index.
 * @param[in] present_queue_family_index The present queue family index.
 * @param[in] shader_stage_cis An array of VkPipelineShaderStageCreateInfo(s).
 * @param[in] shader_stage_ci_count The number of elements in the shader stage array.
 * @param[in/out] swapchain_to_rebuild An existing valid VkSwapchainKHR to rebuild.
 * @param[in/out] surface_to_rebuild An existing valid VkSurfaceKHR to rebuild.
 * @param[in/out] surface_capabilities_to_rebuild An existing valid VkSurfaceCapabilitiesKHR to rebuild.
 * @param[in/out] surface_format_to_rebuild An existing valid VkSurfaceFormatKHR to rebuild.
 * @param[in/out] present_mode_to_rebuild An existing valid VkPresentModeKHR to rebuild.
 * @param[in/out] swapchain_to_rebuild An existing valid VkSwapchainKHR to rebuild.
 *
 * return True on succes, false otherwise.
 */
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
                    u32* const restrict         image_views_rebuilt);

/*
 * @brief Creates a render pass.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] render_pass_create_flags Either 0 or a valid VkRenderPassCreateFlags structure.
 * @param[in] subpass_description_flags Either 0 or a valid VkSubpassDescriptionFlags structure.
 * @param[in] pipeline_bind_point A valid VkPipelineBindPoint.
 * @param[in] attachment_descriptions Either NULL or an array of VkAttachmentDescription structures.
 * @param[in] attachment_description_count The number of elements in the  VkAttachmentDescription array.
 * @param[in] input_attachment_references Either NULL or an array of input attachment references.
 * @param[in] input_attachment_reference_count The number of elements in the input attachment reference
 *            array.
 * @param[in] color_attachment_references Either NULL or an array of color attachment references.
 * @param[in] resolve_attachment_references Either NULL or an array of resolve attachment references.
 * @param[in] color_attachment_reference_count The number of elements in the color attachment reference
 *            array. If the resolve array is non null, this is also the number of resolve attachments.
 * @param[in] preserve_attachments Either NULL or an array of preserve attachments.
 * @param[in] preserve_attachment_count The number of elements in the preseve attachment array.
 * @param[in] depth_stencil_attachment_reference Either NULL or a depth stencil attachment reference.
 * @param[in] subpass_dependencies Either NULL or an array of valid VkSubpassDependencies.
 * @param[in] subpass_dependency_count The number of elements in the subpass dependency array.
 * @param[out] return_render_pass A pointer where the render pass created by this function will be stored.
 *
 * @return True on success, false otherwise.
 */
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
                    VkRenderPass* const restrict                return_render_pass);

/*
 * @brief Destroys a valid VkRenderPass.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] render_pass A valid VkRenderPass to destroy.
 */
void
uvkDestroyRenderPass(const VkDevice logical_device, VkRenderPass render_pass);

/*
 * @brief Creates a VkPipeline for graphics and the associated VkPipelineLayout.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] shader_stage_count The number of shader stages in this pipeline.
 * @param[in] shader_stage_ci An array of create infos for shader stages with shader_stage_count elements.
 * @param[in] vertex_input_state_ci A valid VkPipelineVertexInputStateCreateInfo.
 * @param[in] input_assembly_state_ci A valid VkPipelineInputAssemblyStateCreateInfo.
 * @param[in] viewport_state_ci A valid VkViewportStateCreateInfo.
 * @param[in] rasterization_state_ci A valid VkPipelineRasterizationStateCreateInfo.
 * @param[in] multisample_state_ci A valid VkPipelineMultisampleStateCreateInfo.
 * @param[in] depth_stencil_state_ci A valid VkPipelineDepthStencilStateCreateInfo.
 * @param[in] color_blend_state_ci A valid VkPipelineColorBlendStateCreateInfo.
 * @param[in] dynamic_state_ci A valid VkPipelineDynamicStateCreateInfo.
 * @param[in] pipeline_layout_ci A valid VkPipelineLayoutCreateInfo.
 * @param[in] render_pass A valid VkRenderPass.
 * @param[in] subpass_index The index of the subpass in the VkRenderPass.
 * @param[out] return_pipeline_layout The VkPipelineLayout created by this function.
 * @param[out] return_graphics_pipeline The graphics VkPipeline created by this function.
 *
 * @return True on success, false otherwise.
 */
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
                          VkPipeline* const restrict                   return_graphics_pipeline);

/*
 * @brief Destroys a valid VkPipelineLayout.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] pipeline_layout A valid VkPipelineLayout to destroy.
 */
void
uvkDestroyPipelineLayout(const VkDevice logical_device, VkPipelineLayout pipeline_layout);

/*
 * @brief Destroys a valid VkPipeline.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] pipeline A valid VkPipeline to destroy.
 */
void
uvkDestroyPipeline(const VkDevice logical_device, VkPipeline pipeline);

/*
 * @brief Creates one or more VkFrameBuffer(s).
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] image_count The number of VkImageView attachments. One frame buffer is created per image view.
 * @param[in] image_view_attachments An array of VkImageView(s). The pAttachments member of the
 *            VkFrameBufferCreateInfo structure will be set to each index of this parameter for every
 *            VkFramebuffer creation iteration, up to image_count.
 * @param[in] frame_buffer_ci A valid VkFramebufferCreateInfo. The pAttachments member will be set to each
 *            index of the image_view_attachments array for every VkFrameBuffer creation iteration, up to
 *            image count. Thus, the attachmentCount parameter should always be set to 1.
 * @param[out] The frame buffers created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateFrameBuffers(const VkDevice                    logical_device,
                      const u32                         image_count,
                      const VkImageView* const restrict image_view_attachments,
                      const VkFramebufferCreateInfo     frame_buffer_ci,
                      VkFramebuffer* const restrict     return_frame_buffers);

/*
 * @brief Destroys VkFrameBuffer(s).
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] frame_buffers An array of VkFrameBuffer(s) to destroy.
 * @param[in] frame_buffer_count The number of elements in the VkFrameBuffer array.
 */
void
uvkDestroyFrameBuffers(const VkDevice logical_device,
                       VkFramebuffer* frame_buffers,
                       const u32      frame_buffer_count);

/*
 * @brief Allocates and creates a command buffer(s).
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] command_buffer_ai A valid VkCommandBufferAllocateInfo.
 * @param[out] return_command_buffers The command buffer(s) created by this function.
 *
 * @return True on success, false otherwise.
 */
bool
uvkCreateCommandBuffers(const VkDevice                    logical_device,
                        const VkCommandBufferAllocateInfo command_buffer_ai,
                        VkCommandBuffer** const restrict  return_command_buffers);

/*
 * @brief Destroys all command buffers in a command pool.
 *
 * @param[in] logical_device A valid VkDevice.
 * @param[in] command_pool The VkCommandPool to which the VkCommandBuffer(s) belong.
 * @param[in] command_buffers An array of VkCommandBuffer(s) to destroy.
 * @param[in] command_buffer_count The number of elements in the VkCommandBuffer array.
 *
 * @return True on success, false otherwise.
 */
bool
uvkDestroyCommandBuffers(const VkDevice      logical_device,
                         const VkCommandPool command_pool,
                         VkCommandBuffer*    command_buffers,
                         const u32           command_buffer_count);

/*
 * @brief Prepares a command buffer to begin recording commands.
 *
 * @param[in] command_buffer A valid VkCommandBuffer to record commands on.
 * @param[in] commaned_buffer_bi A valid VkCommandBufferBeginInfo.
 *
 * @return True on success, false otherwise.
 */
bool
uvkBeginCommandBuffer(VkCommandBuffer                command_buffer,
                      const VkCommandBufferBeginInfo command_buffer_bi);

/*
 * @brief Prepares a command buffer to stop recording commands.
 *
 * @param[in] command_buffer A valid VkCommandBuffer to signal.
 *
 * @return True on success, false otherwise.
 */
bool
uvkEndCommandBuffer(VkCommandBuffer command_buffer);

#endif // __UE_VULKAN_TOOLS_H__
