#include "Vulkan.hpp"
#include "GameWindow.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"
GfxInterface *Vulkan::Init()
{
    SDL_Init(SDL_INIT_VIDEO);
    if (!SDL_Vulkan_LoadLibrary(NULL))
    {
        utils::DebugPrint("Failed to load Vulkan library: %s", SDL_GetError());
        return NULL;
    }

    u32 flags = SDL_WINDOW_VULKAN;
    i32 height = GAME_WINDOW_HEIGHT_REAL;
    i32 width = GAME_WINDOW_WIDTH_REAL;
    i32 x = SDL_WINDOWPOS_UNDEFINED;
    i32 y = SDL_WINDOWPOS_UNDEFINED;

    if (g_Supervisor.cfg.windowed == 0)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    Vulkan *self = new Vulkan();

    SDL_Window *window = SDL_CreateWindow(TH_WINDOW_TITLE, x, y, width, height, flags);
    self->window = window;
    if (window == NULL)
    {
        delete self;
        return NULL;
    }

    // create instance

    u32 extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, NULL);
    const char **extensionNames = new const char *[extensionCount];
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensionNames);
    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = TH_WINDOW_TITLE,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "ZUNgine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo instanceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensionNames,
    };

    vkCreateInstance(&instanceCreateInfo, NULL, &self->vkInstance);

    // create debug

    // create surface
    if (!SDL_Vulkan_CreateSurface(window, self->vkInstance, &self->vkSurface))
    {
        utils::DebugPrint("Failed to create Vulkan surface: %s", SDL_GetError());
        delete self;
        return NULL;
    };

    // select physical device

    std::vector<VkPhysicalDevice> physicalDevices;
    u32 physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(self->vkInstance, &physicalDeviceCount, NULL);
    physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(self->vkInstance, &physicalDeviceCount, physicalDevices.data());

    self->vkPhysicalDevice = physicalDevices[0];

    // select queue family

    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    u32 queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(self->vkPhysicalDevice, &queueFamilyCount, nullptr);
    queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(self->vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    int graphicIndex = -1;
    int presentIndex = -1;

    int i = 0;
    for (const auto &queueFamily : queueFamilyProperties)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicIndex = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(self->vkPhysicalDevice, i, self->vkSurface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport)
        {
            presentIndex = i;
        }

        if (graphicIndex != -1 && presentIndex != -1)
        {
            break;
        }

        i++;
    }

    self->graphics_QueueFamilyIndex = graphicIndex;
    self->present_QueueFamilyIndex = presentIndex;

    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const float queue_priority[] = {1.0f};

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = {self->graphics_QueueFamilyIndex, self->present_QueueFamilyIndex};

    float queuePriority = queue_priority[0];
    for (int queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = self->graphics_QueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // https://en.wikipedia.org/wiki/Anisotropic_filtering
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .queueCreateInfoCount = queueCreateInfos.size(),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = deviceExtensions.size(),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures
    };


    //createInfo.enabledLayerCount = validationLayers.size();
    //createInfo.ppEnabledLayerNames = validationLayers.data();

    vkCreateDevice(self->vkPhysicalDevice, &deviceCreateInfo, nullptr, &self->vkDevice);

    vkGetDeviceQueue(self->vkDevice, self->graphics_QueueFamilyIndex, 0, &self->vkGraphicsQueue);
    vkGetDeviceQueue(self->vkDevice, self->present_QueueFamilyIndex, 0, &self->vkPresentQueue);


    //create screen rendering stuff

    //swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .surface = self->vkSurface,
        .minImageCount = 2,
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = {GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    if(!vkCreateSwapchainKHR(self->vkDevice, &swapchainCreateInfo, nullptr, &self->vkSwapchain))
    {
        utils::DebugPrint("Failed to create Vulkan swapchain: %s", SDL_GetError());
        delete self;
        return NULL;
    }

    //image views

    vkGetSwapchainImagesKHR(self->vkDevice, self->vkSwapchain, &self->vkSwapchainImageCount, NULL);
    self->vkSwapchainImages.resize(self->vkSwapchainImageCount);
    vkGetSwapchainImagesKHR(self->vkDevice, self->vkSwapchain, &self->vkSwapchainImageCount, self->vkSwapchainImages.data());

    for(int i = i; i < self->vkSwapchainImageCount; i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .image = self->vkSwapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_UNORM,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if(!vkCreateImageView(self->vkDevice, &imageViewCreateInfo, nullptr, &self->vkSwapchainImageViews[i]))
        {
            utils::DebugPrint("Failed to create Vulkan image view: %s", SDL_GetError());
            delete self;
            return NULL;
        }
    }

    return self;
}

void Vulkan::Exit()
{
    if (this->vkInstance)
    {
        vkDestroyInstance(this->vkInstance, NULL);
        this->vkInstance = NULL;
    }
    if (this->vkSurface)
    {
        vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, NULL);
        this->vkSurface = NULL;
    }
    if (this->window)
    {
        SDL_DestroyWindow(this->window);
        this->window = NULL;
    }
}