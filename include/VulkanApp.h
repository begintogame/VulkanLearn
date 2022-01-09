#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

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

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() { createInstance(); }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
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