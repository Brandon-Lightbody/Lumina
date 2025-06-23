#include <iostream>
#include <vector>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <functional>
#include <array>
#include <chrono>
#include <unordered_map>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;
const int MAX_FRAMES_IN_FLIGHT = 2;
const int MAX_LIGHTS = 1024;
const int CLUSTER_X = 16;
const int CLUSTER_Y = 8;
const int CLUSTER_Z = 24;
const int CLUSTER_COUNT = CLUSTER_X * CLUSTER_Y * CLUSTER_Z;
const float Z_NEAR = 0.1f;
const float Z_FAR = 100.0f;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#include <glm/glm.hpp>
#include <functional>

namespace std {
    template<>
    struct hash<glm::vec2> {
        size_t operator()(const glm::vec2& vec) const noexcept {
            size_t seed = 0;
            hash<float> hasher;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    template<>
    struct hash<glm::vec3> {
        size_t operator()(const glm::vec3& vec) const noexcept {
            size_t seed = 0;
            // Combine hashes of each component
            hash<float> hasher;
            seed ^= hasher(vec.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(vec.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 viewPos;
};

struct Light {
    alignas(16) glm::vec4 position;  // w = radius
    alignas(16) glm::vec4 color;     // w = intensity
};

struct ClusterGrid {
    alignas(16) glm::vec4 screenToView; // [0]: 2.0 / width, [1]: 2.0 / height, [2]: -1.0, [3]: -1.0
    alignas(4) uint32_t clusterDimX;
    alignas(4) uint32_t clusterDimY;
    alignas(4) uint32_t clusterDimZ;
    alignas(4) float zNear;
    alignas(4) float zFar;
    alignas(4) float scale;
    alignas(4) float bias;
};

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 60.0f;

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 getProjectionMatrix(float aspectRatio) {
        return glm::perspective(glm::radians(fov), aspectRatio, Z_NEAR, Z_FAR);
    }
};

class VulkanRenderer {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkCommandPool computeCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandBuffer computeCommandBuffer;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    VkSemaphore computeFinishedSemaphore;
    std::vector<VkFence> inFlightFences;
    VkFence computeFence;
    bool framebufferResized = false;
    size_t currentFrame = 0;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkBuffer lightBuffer;
    VkDeviceMemory lightBufferMemory;
    void* lightBufferMapped;
    VkBuffer clusterGridBuffer;
    VkDeviceMemory clusterGridBufferMemory;
    void* clusterGridBufferMapped;
    VkBuffer clusterLightGridBuffer;
    VkDeviceMemory clusterLightGridBufferMemory;
    VkBuffer clusterLightIndexBuffer;
    VkDeviceMemory clusterLightIndexBufferMemory;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool computeDescriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    VkDescriptorSet computeDescriptorSet;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Camera camera;
    double lastTime = 0.0;
    float rotationAngle = 0.0f;
    VkPipeline computePipeline;
    VkPipelineLayout computePipelineLayout;
    std::vector<Light> lights;
    uint32_t lightCount = 0;
    float timeAccumulator = 0.0f;

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Clustered Shading", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        lastTime = glfwGetTime();
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayouts();
        createGraphicsPipeline();
        createComputePipeline();
        createCommandPools();
        createDepthResources();
        createFramebuffers();
        createSphere();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createLightBuffer();
        createClusterBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createComputeCommandBuffer();
        createSyncObjects();
        addRandomLights(100);
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            update();
            drawFrame();
        }
        vkDeviceWaitIdle(device);
    }

    void update() {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Rotate model
        rotationAngle += 30.0f * deltaTime;
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

        // Update camera
        camera.position.x = sin(glfwGetTime() * 0.3f) * 3.0f;
        camera.position.z = cos(glfwGetTime() * 0.3f) * 3.0f;
        camera.position.y = 1.0f;

        // Update lights
        timeAccumulator += deltaTime;
        if (timeAccumulator > 0.1f && lightCount < MAX_LIGHTS - 10) {
            timeAccumulator = 0.0f;
            addRandomLights(10);
        }
    }

    void addRandomLights(int count) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
        std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
        std::uniform_real_distribution<float> radiusDist(0.5f, 2.0f);
        std::uniform_real_distribution<float> intensityDist(1.0f, 10.0f);

        for (int i = 0; i < count; i++) {
            Light light;
            light.position = glm::vec4(
                posDist(gen),
                posDist(gen) * 0.5f + 1.0f,
                posDist(gen),
                radiusDist(gen)
            );
            light.color = glm::vec4(
                colorDist(gen),
                colorDist(gen),
                colorDist(gen),
                intensityDist(gen)
            );
            lights.push_back(light);
            lightCount++;
        }

        // Update light buffer
        memcpy(lightBufferMapped, lights.data(), sizeof(Light) * lightCount);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, computeDescriptorSetLayout, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyBuffer(device, lightBuffer, nullptr);
        vkFreeMemory(device, lightBufferMemory, nullptr);

        vkDestroyBuffer(device, clusterGridBuffer, nullptr);
        vkFreeMemory(device, clusterGridBufferMemory, nullptr);

        vkDestroyBuffer(device, clusterLightGridBuffer, nullptr);
        vkFreeMemory(device, clusterLightGridBufferMemory, nullptr);

        vkDestroyBuffer(device, clusterLightIndexBuffer, nullptr);
        vkFreeMemory(device, clusterLightIndexBufferMemory, nullptr);

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorPool(device, computeDescriptorPool, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroySemaphore(device, computeFinishedSemaphore, nullptr);
        vkDestroyFence(device, computeFence, nullptr);

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, computeCommandPool, nullptr);
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipeline(device, computePipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipelineLayout(device, computePipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan Clustered Shading";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Custom Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = &debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.computeFamily.value(),
            indices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {
            indices.graphicsFamily.value(),
            indices.computeFamily.value(),
            indices.presentFamily.value()
        };

        if (indices.graphicsFamily != indices.presentFamily ||
            indices.graphicsFamily != indices.computeFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 3;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views!");
            }
        }
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    void createDescriptorSetLayouts() {
        // Graphics descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> bindings = {
            // UBO
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            // Light buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            // Cluster grid
            {2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            // Cluster light grid
            {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            // Cluster light index list
            {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
        };

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        // Compute descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> computeBindings = {
            // Cluster grid
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            // Light buffer
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            // Cluster light grid
            {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
            // Cluster light index list
            {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}
        };

        VkDescriptorSetLayoutCreateInfo computeLayoutInfo{};
        computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        computeLayoutInfo.bindingCount = static_cast<uint32_t>(computeBindings.size());
        computeLayoutInfo.pBindings = computeBindings.data();

        if (vkCreateDescriptorSetLayout(device, &computeLayoutInfo, nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute descriptor set layout!");
        }
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = compileShaderToSPV(
            "#version 450\n"
            "layout(location = 0) in vec3 inPosition;\n"
            "layout(location = 1) in vec3 inNormal;\n"
            "layout(location = 2) in vec2 inTexCoord;\n"
            "layout(location = 0) out vec3 fragPos;\n"
            "layout(location = 1) out vec3 fragNormal;\n"
            "layout(location = 2) out vec2 fragTexCoord;\n"
            "layout(binding = 0) uniform UniformBufferObject {\n"
            "    mat4 model;\n"
            "    mat4 view;\n"
            "    mat4 proj;\n"
            "    vec3 viewPos;\n"
            "} ubo;\n"
            "void main() {\n"
            "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);\n"
            "    fragPos = vec3(ubo.model * vec4(inPosition, 1.0));\n"
            "    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;\n"
            "    fragTexCoord = inTexCoord;\n"
            "}", VK_SHADER_STAGE_VERTEX_BIT);

        auto fragShaderCode = compileShaderToSPV(
            "#version 450\n"
            "layout(location = 0) in vec3 fragPos;\n"
            "layout(location = 1) in vec3 fragNormal;\n"
            "layout(location = 2) in vec2 fragTexCoord;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "struct Light {\n"
            "    vec4 position;\n"
            "    vec4 color;\n"
            "};\n"
            "layout(binding = 0) uniform UniformBufferObject {\n"
            "    mat4 model;\n"
            "    mat4 view;\n"
            "    mat4 proj;\n"
            "    vec3 viewPos;\n"
            "} ubo;\n"
            "layout(binding = 1) buffer LightBuffer {\n"
            "    Light lights[];\n"
            "};\n"
            "layout(binding = 2) uniform ClusterGrid {\n"
            "    vec4 screenToView;\n"
            "    uint clusterDimX;\n"
            "    uint clusterDimY;\n"
            "    uint clusterDimZ;\n"
            "    float zNear;\n"
            "    float zFar;\n"
            "    float scale;\n"
            "    float bias;\n"
            "} clusterGrid;\n"
            "layout(binding = 3) buffer ClusterLightGrid {\n"
            "    uint lightGrid[];\n"
            "};\n"
            "layout(binding = 4) buffer ClusterLightIndexList {\n"
            "    uint lightIndexList[];\n"
            "};\n"
            "uint getClusterIndex(vec2 screenPos, float viewZ) {\n"
            "    uint zSlice = uint(max(log2(viewZ) * clusterGrid.scale + clusterGrid.bias, 0.0));\n"
            "    zSlice = min(zSlice, clusterGrid.clusterDimZ - 1);\n"
            "    uvec3 clusterIndex3D;\n"
            "    clusterIndex3D.x = uint(screenPos.x * clusterGrid.clusterDimX);\n"
            "    clusterIndex3D.y = uint(screenPos.y * clusterGrid.clusterDimY);\n"
            "    clusterIndex3D.z = zSlice;\n"
            "    return clusterIndex3D.x + \n"
            "           clusterIndex3D.y * clusterGrid.clusterDimX + \n"
            "           clusterIndex3D.z * (clusterGrid.clusterDimX * clusterGrid.clusterDimY);\n"
            "}\n"
            "void main() {\n"
            "    vec3 N = normalize(fragNormal);\n"
            "    vec3 V = normalize(ubo.viewPos - fragPos);\n"
            "    vec2 screenPos = gl_FragCoord.xy / vec2(1280, 720);\n"
            "    float viewZ = length(ubo.viewPos - fragPos);\n"
            "    uint clusterIndex = getClusterIndex(screenPos, viewZ);\n"
            "    uint lightCount = lightGrid[clusterIndex * 2];\n"
            "    uint lightOffset = lightGrid[clusterIndex * 2 + 1];\n"
            "    vec3 materialColor = vec3(0.8, 0.8, 0.8);\n"
            "    vec3 ambient = materialColor * 0.1;\n"
            "    vec3 lighting = ambient;\n"
            "    for (uint i = 0; i < lightCount; i++) {\n"
            "        uint lightIndex = lightIndexList[lightOffset + i];\n"
            "        Light light = lights[lightIndex];\n"
            "        vec3 L = normalize(light.position.xyz - fragPos);\n"
            "        vec3 H = normalize(V + L);\n"
            "        float diff = max(dot(N, L), 0.0);\n"
            "        vec3 diffuse = diff * materialColor * light.color.rgb * light.color.a;\n"
            "        float spec = pow(max(dot(N, H), 0.0), 32.0);\n"
            "        vec3 specular = spec * light.color.rgb * light.color.a;\n"
            "        float dist = length(light.position.xyz - fragPos);\n"
            "        float attenuation = 1.0 / (1.0 + 0.1 * dist + 0.01 * dist * dist);\n"
            "        lighting += (diffuse + specular) * attenuation;\n"
            "    }\n"
            "    outColor = vec4(lighting, 1.0);\n"
            "}", VK_SHADER_STAGE_FRAGMENT_BIT);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createComputePipeline() {
        auto computeShaderCode = compileShaderToSPV(
            "#version 450\n"
            "struct Light {\n"
            "    vec4 position;\n"
            "    vec4 color;\n"
            "};\n"
            "layout(binding = 0) uniform ClusterGrid {\n"
            "    vec4 screenToView;\n"
            "    uint clusterDimX;\n"
            "    uint clusterDimY;\n"
            "    uint clusterDimZ;\n"
            "    float zNear;\n"
            "    float zFar;\n"
            "    float scale;\n"
            "    float bias;\n"
            "} clusterGrid;\n"
            "layout(binding = 1) buffer LightBuffer {\n"
            "    Light lights[];\n"
            "};\n"
            "layout(binding = 2) buffer ClusterLightGrid {\n"
            "    uint lightGrid[];\n"
            "};\n"
            "layout(binding = 3) buffer ClusterLightIndexList {\n"
            "    uint lightIndexList[];\n"
            "};\n"
            "layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;\n"
            "void getClusterAABB(uint clusterIndex, out vec3 minPoint, out vec3 maxPoint) {\n"
            "    uint zSlice = clusterIndex / (clusterGrid.clusterDimX * clusterGrid.clusterDimY);\n"
            "    uint ySlice = (clusterIndex % (clusterGrid.clusterDimX * clusterGrid.clusterDimY)) / clusterGrid.clusterDimX;\n"
            "    uint xSlice = clusterIndex % clusterGrid.clusterDimX;\n"
            "    minPoint.x = float(xSlice) / float(clusterGrid.clusterDimX);\n"
            "    minPoint.y = float(ySlice) / float(clusterGrid.clusterDimY);\n"
            "    minPoint.z = exp2((float(zSlice) - clusterGrid.bias) / clusterGrid.scale) * clusterGrid.zNear;\n"
            "    maxPoint.x = float(xSlice + 1) / float(clusterGrid.clusterDimX);\n"
            "    maxPoint.y = float(ySlice + 1) / float(clusterGrid.clusterDimY);\n"
            "    maxPoint.z = exp2((float(zSlice + 1) - clusterGrid.bias) / clusterGrid.scale) * clusterGrid.zNear;\n"
            "    minPoint.xy = minPoint.xy * 2.0 - 1.0;\n"
            "    maxPoint.xy = maxPoint.xy * 2.0 - 1.0;\n"
            "    minPoint.z = -minPoint.z;\n"
            "    maxPoint.z = -maxPoint.z;\n"
            "}\n"
            "bool sphereVsAABB(vec3 center, float radius, vec3 minPoint, vec3 maxPoint) {\n"
            "    vec3 closest = clamp(center, minPoint, maxPoint);\n"
            "    float dist = length(center - closest);\n"
            "    return dist < radius;\n"
            "}\n"
            "void main() {\n"
            "    uint clusterIndex = gl_GlobalInvocationID.x + \n"
            "                        gl_GlobalInvocationID.y * clusterGrid.clusterDimX + \n"
            "                        gl_GlobalInvocationID.z * clusterGrid.clusterDimX * clusterGrid.clusterDimY;\n"
            "    lightGrid[clusterIndex * 2] = 0;\n"
            "    vec3 minPoint, maxPoint;\n"
            "    getClusterAABB(clusterIndex, minPoint, maxPoint);\n"
            "    uint globalOffset = clusterIndex * 1024;\n"
            "    for (uint i = 0; i < lights.length(); i++) {\n"
            "        Light light = lights[i];\n"
            "        vec3 lightPos = light.position.xyz;\n"
            "        float radius = light.position.w;\n"
            "        if (sphereVsAABB(lightPos, radius, minPoint, maxPoint)) {\n"
            "            uint index = atomicAdd(lightGrid[clusterIndex * 2], 1);\n"
            "            lightIndexList[globalOffset + index] = i;\n"
            "        }\n"
            "    }\n"
            "}", VK_SHADER_STAGE_COMPUTE_BIT);

        VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShaderModule;
        computeShaderStageInfo.pName = "main";

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = computeShaderStageInfo;
        pipelineInfo.layout = computePipelineLayout;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute pipeline!");
        }

        vkDestroyShaderModule(device, computeShaderModule, nullptr);
    }

    void createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void createCommandPools() {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        // Graphics command pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }

        // Compute command pool
        poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute command pool!");
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();

        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    void createSphere() {
        const int X_SEGMENTS = 64;
        const int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;

        for (int y = 0; y <= Y_SEGMENTS; ++y) {
            for (int x = 0; x <= X_SEGMENTS; ++x) {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                Vertex vertex;
                vertex.pos = glm::vec3(xPos, yPos, zPos);
                vertex.normal = glm::vec3(xPos, yPos, zPos);
                vertex.texCoord = glm::vec2(xSegment, ySegment);
                vertices.push_back(vertex);
            }
        }

        for (int y = 0; y < Y_SEGMENTS; ++y) {
            for (int x = 0; x < X_SEGMENTS; ++x) {
                int first = (y * (X_SEGMENTS + 1)) + x;
                int second = first + X_SEGMENTS + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
    }

    void createVertexBuffer() {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createLightBuffer() {
        VkDeviceSize bufferSize = sizeof(Light) * MAX_LIGHTS;

        createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            lightBuffer, lightBufferMemory);

        vkMapMemory(device, lightBufferMemory, 0, bufferSize, 0, &lightBufferMapped);
    }

    void createClusterBuffers() {
        // Cluster grid buffer
        VkDeviceSize gridBufferSize = sizeof(ClusterGrid);
        createBuffer(gridBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            clusterGridBuffer, clusterGridBufferMemory);

        vkMapMemory(device, clusterGridBufferMemory, 0, gridBufferSize, 0, &clusterGridBufferMapped);

        // Update cluster grid data
        ClusterGrid clusterGrid;
        clusterGrid.screenToView = glm::vec4(
            2.0f / swapChainExtent.width,
            2.0f / swapChainExtent.height,
            -1.0f,
            -1.0f
        );
        clusterGrid.clusterDimX = CLUSTER_X;
        clusterGrid.clusterDimY = CLUSTER_Y;
        clusterGrid.clusterDimZ = CLUSTER_Z;
        clusterGrid.zNear = Z_NEAR;
        clusterGrid.zFar = Z_FAR;
        clusterGrid.scale = float(CLUSTER_Z) / log2(clusterGrid.zFar / clusterGrid.zNear);
        clusterGrid.bias = -(float(CLUSTER_Z) * log2(clusterGrid.zNear) / log2(clusterGrid.zFar / clusterGrid.zNear));

        memcpy(clusterGridBufferMapped, &clusterGrid, sizeof(ClusterGrid));

        // Cluster light grid buffer
        VkDeviceSize lightGridBufferSize = sizeof(uint32_t) * 2 * CLUSTER_COUNT;
        createBuffer(lightGridBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            clusterLightGridBuffer, clusterLightGridBufferMemory);

        // Cluster light index list buffer
        VkDeviceSize lightIndexBufferSize = sizeof(uint32_t) * MAX_LIGHTS * CLUSTER_COUNT;
        createBuffer(lightIndexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            clusterLightIndexBuffer, clusterLightIndexBufferMemory);
    }

    void createDescriptorPool() {
        // Graphics descriptor pool
        std::array<VkDescriptorPoolSize, 5> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }

        // Compute descriptor pool
        std::array<VkDescriptorPoolSize, 4> computePoolSizes{};
        computePoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        computePoolSizes[0].descriptorCount = 1;
        computePoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        computePoolSizes[1].descriptorCount = 1;
        computePoolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        computePoolSizes[2].descriptorCount = 1;
        computePoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        computePoolSizes[3].descriptorCount = 1;

        VkDescriptorPoolCreateInfo computePoolInfo{};
        computePoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        computePoolInfo.poolSizeCount = static_cast<uint32_t>(computePoolSizes.size());
        computePoolInfo.pPoolSizes = computePoolSizes.data();
        computePoolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(device, &computePoolInfo, nullptr, &computeDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute descriptor pool!");
        }
    }

    void createDescriptorSets() {
        // Graphics descriptor sets
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo uniformBufferInfo{};
            uniformBufferInfo.buffer = uniformBuffers[i];
            uniformBufferInfo.offset = 0;
            uniformBufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorBufferInfo lightBufferInfo{};
            lightBufferInfo.buffer = lightBuffer;
            lightBufferInfo.offset = 0;
            lightBufferInfo.range = sizeof(Light) * MAX_LIGHTS;

            VkDescriptorBufferInfo clusterGridInfo{};
            clusterGridInfo.buffer = clusterGridBuffer;
            clusterGridInfo.offset = 0;
            clusterGridInfo.range = sizeof(ClusterGrid);

            VkDescriptorBufferInfo clusterLightGridInfo{};
            clusterLightGridInfo.buffer = clusterLightGridBuffer;
            clusterLightGridInfo.offset = 0;
            clusterLightGridInfo.range = sizeof(uint32_t) * 2 * CLUSTER_COUNT;

            VkDescriptorBufferInfo clusterLightIndexInfo{};
            clusterLightIndexInfo.buffer = clusterLightIndexBuffer;
            clusterLightIndexInfo.offset = 0;
            clusterLightIndexInfo.range = sizeof(uint32_t) * MAX_LIGHTS * CLUSTER_COUNT;

            std::vector<VkWriteDescriptorSet> descriptorWrites = {
                // UBO
                {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[i], 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &uniformBufferInfo, nullptr},
                // Light buffer
                {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[i], 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &lightBufferInfo, nullptr},
                // Cluster grid
                {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[i], 2, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &clusterGridInfo, nullptr},
                // Cluster light grid
                {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[i], 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &clusterLightGridInfo, nullptr},
                // Cluster light index list
                {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[i], 4, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &clusterLightIndexInfo, nullptr}
            };

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);
        }

        // Compute descriptor set
        VkDescriptorSetAllocateInfo computeAllocInfo{};
        computeAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        computeAllocInfo.descriptorPool = computeDescriptorPool;
        computeAllocInfo.descriptorSetCount = 1;
        computeAllocInfo.pSetLayouts = &computeDescriptorSetLayout;

        if (vkAllocateDescriptorSets(device, &computeAllocInfo, &computeDescriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate compute descriptor set!");
        }

        VkDescriptorBufferInfo clusterGridInfo{};
        clusterGridInfo.buffer = clusterGridBuffer;
        clusterGridInfo.offset = 0;
        clusterGridInfo.range = sizeof(ClusterGrid);

        VkDescriptorBufferInfo lightBufferInfo{};
        lightBufferInfo.buffer = lightBuffer;
        lightBufferInfo.offset = 0;
        lightBufferInfo.range = sizeof(Light) * MAX_LIGHTS;

        VkDescriptorBufferInfo clusterLightGridInfo{};
        clusterLightGridInfo.buffer = clusterLightGridBuffer;
        clusterLightGridInfo.offset = 0;
        clusterLightGridInfo.range = sizeof(uint32_t) * 2 * CLUSTER_COUNT;

        VkDescriptorBufferInfo clusterLightIndexInfo{};
        clusterLightIndexInfo.buffer = clusterLightIndexBuffer;
        clusterLightIndexInfo.offset = 0;
        clusterLightIndexInfo.range = sizeof(uint32_t) * MAX_LIGHTS * CLUSTER_COUNT;

        std::vector<VkWriteDescriptorSet> computeDescriptorWrites = {
            // Cluster grid
            {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, computeDescriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &clusterGridInfo, nullptr},
            // Light buffer
            {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, computeDescriptorSet, 1, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &lightBufferInfo, nullptr},
            // Cluster light grid
            {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, computeDescriptorSet, 2, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &clusterLightGridInfo, nullptr},
            // Cluster light index list
            {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, computeDescriptorSet, 3, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &clusterLightIndexInfo, nullptr}
        };

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(computeDescriptorWrites.size()),
            computeDescriptorWrites.data(), 0, nullptr);
    }

    void createCommandBuffers() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void createComputeCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = computeCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate compute command buffer!");
        }

        // Record the compute command buffer once as it doesn't change
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording compute command buffer!");
        }

        vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
        vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &computeDescriptorSet, 0, nullptr);
        vkCmdDispatch(computeCommandBuffer, CLUSTER_X, CLUSTER_Y, CLUSTER_Z);

        if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record compute command buffer!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
            }
        }

        // Compute synchronization objects
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &computeFinishedSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute semaphore!");
        }

        VkFenceCreateInfo computeFenceInfo{};
        computeFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        computeFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(device, &computeFenceInfo, nullptr, &computeFence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create compute fence!");
        }
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        // Run compute pass for light culling
        runComputePass();

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame], computeFinishedSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void runComputePass() {
        vkWaitForFences(device, 1, &computeFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &computeFence);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &computeCommandBuffer;

        if (vkQueueSubmit(computeQueue, 1, &submitInfo, computeFence) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit compute command buffer!");
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), rotationAngle * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.view = camera.getViewMatrix();
        ubo.proj = camera.getProjectionMatrix((float)swapChainExtent.width / (float)swapChainExtent.height);
        ubo.proj[1][1] *= -1; // Flip Y-axis for Vulkan
        ubo.viewPos = camera.position;

        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createDepthResources();
        createFramebuffers();
        updateClusterGrid();
    }

    void updateClusterGrid() {
        ClusterGrid clusterGrid;
        clusterGrid.screenToView = glm::vec4(
            2.0f / swapChainExtent.width,
            2.0f / swapChainExtent.height,
            -1.0f,
            -1.0f
        );
        clusterGrid.clusterDimX = CLUSTER_X;
        clusterGrid.clusterDimY = CLUSTER_Y;
        clusterGrid.clusterDimZ = CLUSTER_Z;
        clusterGrid.zNear = Z_NEAR;
        clusterGrid.zFar = Z_FAR;
        clusterGrid.scale = float(CLUSTER_Z) / log2(clusterGrid.zFar / clusterGrid.zNear);
        clusterGrid.bias = -(float(CLUSTER_Z) * log2(clusterGrid.zNear) / log2(clusterGrid.zFar / clusterGrid.zNear));

        memcpy(clusterGridBufferMapped, &clusterGrid, sizeof(ClusterGrid));
    }

    void cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    // Helper functions for buffer and image creation
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image view!");
        }

        return imageView;
    }

    // Helper structs and functions
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> computeFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && computeFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                indices.computeFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate &&
            supportedFeatures.samplerAnisotropy && supportedFeatures.fragmentStoresAndAtomics;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }
        return true;
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find supported format!");
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type!");
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkShaderModule createShaderModule(const std::vector<uint32_t>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }
        return shaderModule;
    }

    std::vector<uint32_t> compileShaderToSPV(const std::string& source, VkShaderStageFlagBits stage) {
        // For simplicity, we'll just return dummy SPIR-V
        // In a real application, you would use glslang or shaderc to compile
        // For this demo, we'll just return a simple shader
        if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
            return {
                0x07230203,0x00010000,0x00080001,0x0000001e,
                0x00000000,0x00020011,0x00000001,0x0006000b,
                0x00000001,0x4c534c47,0x6474732e,0x3035342e,
                0x00000000,0x0003000e,0x00000000,0x00000001,
                0x0007000f,0x00000000,0x00000004,0x6e69616d,
                0x00000000,0x0000000d,0x00000017,0x00050048,
                0x0000000b,0x00000000,0x0000000b,0x00000000,
                0x00050048,0x0000000b,0x00000001,0x0000000b,
                0x00000001,0x00050048,0x0000000b,0x00000002,
                0x0000000b,0x00000003,0x00050048,0x0000000b,
                0x00000003,0x0000000b,0x00000004,0x00030047,
                0x0000000b,0x00000002,0x00040047,0x0000000d,
                0x0000001e,0x00000000,0x00040047,0x00000017,
                0x0000001e,0x00000000,0x00020013,0x00000002,
                0x00030021,0x00000003,0x00000002,0x00030016,
                0x00000006,0x00000020,0x00040017,0x00000007,
                0x00000006,0x00000004,0x00040015,0x00000008,
                0x00000020,0x00000000,0x0004002b,0x00000008,
                0x00000009,0x00000001,0x0004001c,0x0000000a,
                0x00000006,0x00000009,0x0006001e,0x0000000b,
                0x00000007,0x00000006,0x0000000a,0x0000000a,
                0x00040020,0x0000000c,0x00000003,0x0000000b,
                0x0004003b,0x0000000c,0x0000000d,0x00000003,
                0x00040015,0x0000000e,0x00000020,0x00000001,
                0x0004002b,0x0000000e,0x0000000f,0x00000000,
                0x00040020,0x00000010,0x00000001,0x00000007,
                0x0004003b,0x00000010,0x00000011,0x00000001,
                0x00040020,0x00000014,0x00000003,0x00000007,
                0x0004003b,0x00000014,0x00000017,0x00000003,
                0x0004002b,0x00000006,0x00000019,0x3f800000,
                0x00050036,0x00000002,0x00000004,0x00000000,
                0x00000003,0x000200f8,0x00000005,0x0004003d,
                0x00000007,0x00000012,0x00000011,0x00050051,
                0x00000006,0x00000013,0x00000012,0x00000000,
                0x00050051,0x00000006,0x00000015,0x00000012,
                0x00000001,0x00050051,0x00000006,0x00000016,
                0x00000012,0x00000002,0x00070050,0x00000007,
                0x00000018,0x00000013,0x00000015,0x00000016,
                0x00000019,0x0003003e,0x00000017,0x00000018,
                0x000100fd,0x00010038
            };
        }
        else if (stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            return {
                0x07230203,0x00010000,0x00080001,0x0000001e,
                0x00000000,0x00020011,0x00000001,0x0006000b,
                0x00000001,0x4c534c47,0x6474732e,0x3035342e,
                0x00000000,0x0003000e,0x00000000,0x00000001,
                0x0007000f,0x00000004,0x00000004,0x6e69616d,
                0x00000000,0x00000009,0x0000000d,0x00030010,
                0x00000004,0x00000007,0x00030003,0x00000002,
                0x000001c2,0x00040005,0x00000004,0x6e69616d,
                0x00000000,0x00050005,0x00000009,0x67617266,
                0x6f6c6f43,0x00000072,0x00050005,0x0000000d,
                0x67617266,0x6f6c6f43,0x00000072,0x00040047,
                0x00000009,0x0000001e,0x00000000,0x00040047,
                0x0000000d,0x0000001e,0x00000000,0x00020013,
                0x00000002,0x00030021,0x00000003,0x00000002,
                0x00030016,0x00000006,0x00000020,0x00040017,
                0x00000007,0x00000006,0x00000004,0x00040020,
                0x00000008,0x00000003,0x00000007,0x0004003b,
                0x00000008,0x00000009,0x00000003,0x00040020,
                0x0000000a,0x00000001,0x00000007,0x0004003b,
                0x0000000a,0x0000000d,0x00000001,0x0004002b,
                0x00000006,0x0000000f,0x3f800000,0x00050036,
                0x00000002,0x00000004,0x00000000,0x00000003,
                0x000200f8,0x00000005,0x0004003d,0x00000007,
                0x0000000e,0x0000000d,0x0005008e,0x00000007,
                0x00000010,0x0000000e,0x0000000f,0x0003003e,
                0x00000009,0x00000010,0x000100fd,0x00010038
            };
        }
        else { // Compute shader
            return {
                0x07230203,0x00010000,0x00080001,0x0000001e,
                0x00000000,0x00020011,0x00000001,0x0006000b,
                0x00000001,0x4c534c47,0x6474732e,0x3035342e,
                0x00000000,0x0003000e,0x00000000,0x00000001,
                0x0006000f,0x00000005,0x00000004,0x6e69616d,
                0x00000000,0x0000000d,0x00060010,0x00000004,
                0x00000011,0x00000001,0x00000001,0x00000001,
                0x00030003,0x00000002,0x000001c2,0x00040005,
                0x00000004,0x6e69616d,0x00000000,0x00050005,
                0x00000009,0x76646172,0x6f635f65,0x0064726f,
                0x00060006,0x00000009,0x00000000,0x5f6c6c75,
                0x6f6c6f43,0x00000072,0x00060006,0x00000009,
                0x00000001,0x6e617274,0x726f6673,0x0000006d,
                0x00060006,0x00000009,0x00000002,0x5f6c6c75,
                0x6f6c6f43,0x00000072,0x00060006,0x00000009,
                0x00000003,0x5f6c6c75,0x6f6c6f43,0x00000072,
                0x00040005,0x0000000b,0x76646172,0x00000000,
                0x00050005,0x0000000d,0x76646172,0x6f635f65,
                0x0064726f,0x00050048,0x00000009,0x00000000,
                0x00000023,0x00000000,0x00050048,0x00000009,
                0x00000001,0x00000023,0x00000004,0x00050048,
                0x00000009,0x00000002,0x00000023,0x00000008,
                0x00050048,0x00000009,0x00000003,0x00000023,
                0x0000000c,0x00030047,0x00000009,0x00000002,
                0x00040047,0x0000000d,0x00000022,0x00000000,
                0x00040047,0x0000000d,0x00000021,0x00000000,
                0x00020013,0x00000002,0x00030021,0x00000003,
                0x00000002,0x00030016,0x00000006,0x00000020,
                0x00040017,0x00000007,0x00000006,0x00000004,
                0x00040018,0x00000008,0x00000007,0x00000004,
                0x0004001e,0x00000009,0x00000008,0x00000006,
                0x00040020,0x0000000a,0x00000009,0x00000009,
                0x0004003b,0x0000000a,0x0000000b,0x00000009,
                0x00040015,0x0000000c,0x00000020,0x00000001,
                0x00090019,0x0000000e,0x0000000c,0x00000006,
                0x00000000,0x00000000,0x00000000,0x00000002,
                0x00000000,0x00040020,0x0000000f,0x00000000,
                0x0000000e,0x0004003b,0x0000000f,0x00000010,
                0x00000000,0x00040017,0x00000012,0x0000000c,
                0x00000003,0x00040020,0x00000013,0x00000001,
                0x00000012,0x0004003b,0x00000013,0x00000014,
                0x00000001,0x0004002b,0x0000000c,0x00000016,
                0x00000000,0x00040020,0x00000017,0x00000009,
                0x00000008,0x00050036,0x00000002,0x00000004,
                0x00000000,0x00000003,0x000200f8,0x00000005,
                0x000100fd,0x00010038
            };
        }
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
};

int main() {
    VulkanRenderer app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}