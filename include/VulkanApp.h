#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        auto enabledExtensionCount = glfwExtensionCount;
        auto ppEnabledExtensionNames = glfwExtensions;

        vk::InstanceCreateInfo createInfo({}, &appInfo, enabledLayerCount, nullptr,
                                          enabledExtensionCount, ppEnabledExtensionNames);
        // create an Instance
        instance = vk::createInstance(createInfo);
    }
};