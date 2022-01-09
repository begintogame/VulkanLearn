#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() { return graphicsFamily.has_value(); }
};

class HelloTriangleApplication {
  public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    GLFWwindow* window;

    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;

    vk::Device device;
    vk::Queue graphicsQueue;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        device.destroy();
        instance.destroy();

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance() {
        // initialize the vk::ApplicationInfo structure
        auto sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        auto pApplicationName = "Hello Triangle";
        auto applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        auto pEngineName = "No Engine";
        auto engineVersion = VK_MAKE_VERSION(1, 0, 0);
        auto apiVersion = VK_API_VERSION_1_1;

        vk::ApplicationInfo appInfo(pApplicationName,
                                    applicationVersion,
                                    pEngineName,
                                    engineVersion,
                                    apiVersion);

        // initialize the vk::InstanceCreateInfo
        auto enabledLayerCount = 0;

        const char* const* ppEnabledLayerNames = nullptr;

        auto extensions = getRequiredExtensions();

        void* pNext = nullptr;

#if !defined(NDEBUG)
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                                          | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                          | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                                      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        debugCreateInfo.pfnUserCallback = debugCallback;
        pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        enabledLayerCount = validationLayers.size();
        ppEnabledLayerNames = validationLayers.data();

#endif

        auto enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        auto ppEnabledExtensionNames = extensions.data();

        vk::InstanceCreateInfo createInfo({}, &appInfo, enabledLayerCount, ppEnabledLayerNames,
                                          enabledExtensionCount, ppEnabledExtensionNames);
        createInfo.setPNext(pNext);
        // create an Instance
        instance = vk::createInstance(createInfo);
    }
    void pickPhysicalDevice() {
        auto devices = instance.enumeratePhysicalDevices();

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                return;
            }
        }

        throw std::runtime_error("failed to find a suitable GPU!");
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        // create a Device
        auto queueFamilyIndex = indices.graphicsFamily.value();
        auto queueCount = 1;
        float queuePriority = 1.0f;

        vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamilyIndex,
                                                  queueCount, &queuePriority);

        vk::PhysicalDeviceFeatures deviceFeatures{};

        uint32_t queueCreateInfoCount = 1;
        auto pEnabledFeatures = &deviceFeatures;
        uint32_t enabledExtensionCount = 0;
        uint32_t enabledLayerCount = 0;
        const char*const* ppEnabledLayerNames = nullptr;
#if !defined(NDEBUG) 
        enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        ppEnabledLayerNames = validationLayers.data();
#endif
        vk::DeviceCreateInfo createInfo(vk::DeviceCreateFlags(), queueCreateInfoCount,
                                        &queueCreateInfo, enabledLayerCount, ppEnabledLayerNames,
                                        enabledExtensionCount, nullptr, pEnabledFeatures);
        device = physicalDevice.createDevice(createInfo);
        graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        return indices.isComplete();
    }

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) {
        QueueFamilyIndices indices;

        auto queueFamilies = device.getQueueFamilyProperties();

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        return extensions;
    }
#if !defined(NDEBUG)
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
#endif
};