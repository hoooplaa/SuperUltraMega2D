#pragma once

#define VK_KHR_SWAPCHAIN_EXTENSION_NAME_MINE "VK_KHR_swapchain"

#ifdef NDEBUG
const bool g_enableValidationLayers = false;
const bool g_isDebugMode = false;
#else
const bool g_enableValidationLayers = true;
const bool g_isDebugMode = true;
#endif

#include <unordered_set>
#include <unordered_map>

#include "VulkanInclude.h"
#include "VulkanObjects.h"
#include "VulkanImgui.h"

#include "Engine/Graphics/Objects/Camera.h"
#include "Engine/Graphics/Objects/Batch.h"

namespace Mega { class Renderer; }

namespace std {
	template<> struct hash<Mega::Vertex> {
		size_t operator()(Mega::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct VertexData {
	uint32_t indices[2] = { 0, 0 };
};

enum class eVulkanInitState {
	Created,
	Initialzed,
	Destroyed
};

class Vulkan {
private:
	friend Mega::Renderer;
	friend ImguiObject;

	VertexData* m_pBoxVertexData;

	void Initialize(Mega::Renderer* in_pRenderer, GLFWwindow* in_pWindow);
	void Destroy();
	void RecreateSwapchain();
	void CleanupSwapchain(VkSwapchainKHR* in_pSwapchain);

	void DrawFrame(std::vector<Mega::Batch>& in_batches);

	void SetViewData(const Mega::Camera::ViewData& in_viewData);

	void LoadVertexData(const char* in_objPath, VertexData* in_pVertexData);
	void LoadTextureData(const char* in_texPath, Mega::Texture* in_pTextureData);

	void UpdateLoadedVertexData(std::vector<Mega::Batch>& in_batches);
	void UpdateLoadedIndexData();
	void UpdateLoadedTextureData();

private:
	// Setup
	void CreateInstance();
	bool CheckValidationLayerSupport();
	bool CheckGLFWExtensionSupport();

	void CreateWin32SurfaceKHR(GLFWwindow* in_pWindow, VkSurfaceKHR& in_surface);

	void PickPhysicalDevice(VkPhysicalDevice& in_device);
	QueueFamilyIndices FindQueueFamilies(const VkPhysicalDevice in_device, VkSurfaceKHR in_surface);
	bool IsPhysicalDeviceSuitable(const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface);
	bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice in_device);
	SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface);

	void CreateLogicalDevice(VkDevice& in_device);

	void CreateSwapchain(GLFWwindow* in_pWindow, const VkSurfaceKHR in_surface, VkSwapchainKHR& in_swapchain);

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& in_availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& in_availableModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& in_capabilities, GLFWwindow* in_pWindow);

	void RetrieveSwapchainImages(std::vector<VkImage>& in_images, VkSwapchainKHR in_swapchain);
	void CreateSwapchainImageViews(std::vector<VkImageView>& in_imageViews, const std::vector<VkImage>& in_images);

	void CreateDescriptorSetLayout(const VkDevice in_device, VkDescriptorSetLayout& in_descriptorSetLayout);
	void CreateRenderPass();
	void CreateGraphicsPipeline(VkShaderModule& in_vertShaderModule, VkShaderModule& in_fragShaderModule, VkPipeline& in_pipeline);

	void CreateFramebuffers(std::vector<VkFramebuffer>& in_swapchainFramebuffers);

	void CreateDrawCommandPools(std::vector<VkCommandPool>& in_pools);
	void CreateDrawCommands(std::vector<VkCommandBuffer>& in_buffers, std::vector<VkCommandPool>& in_pools);
	void AllocateCommandBuffer(VkCommandBuffer& in_buffer, const VkCommandPool& in_pool);

	VkCommandBuffer BeginSingleTimeCommand(const VkCommandPool& in_pool);
	void EndSingleTimeCommand(const VkCommandPool& in_pool, VkCommandBuffer in_command);

	void CreateDepthResources(ImageObject& in_depthObject);
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& in_candidates, VkImageTiling in_tiling, VkFormatFeatureFlags in_features);
	VkFormat FindDepthFormat();
	bool HasStencilComponent(VkFormat format);

	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void UpdateDescriptorSets();

	void CreateVertexBuffer(std::vector<Mega::Vertex>& in_vertices, VkBuffer& in_buffer, VkDeviceMemory& in_memory);
	void CreateIndexBuffer(std::vector<INDEX_TYPE>& in_indices, VkBuffer& in_buffer, VkDeviceMemory& in_memory);

	void CreateUniformBuffers();
	void UpdateUniformBuffer(uint32_t in_imageIndex);

	void CreateSyncObjects();

	// Other
	void CreateTextureSampler(VkSampler in_sampler);
	void CreateTextureImage(ImageObject& in_imageObject, const char* in_filename);

	void CreateImageObject(VkImageCreateInfo& in_info, VkMemoryPropertyFlags in_properties, VkImage& in_image, VkDeviceMemory& in_imageMemory);
	void ChangeImageLayout(VkImage& in_image, VkFormat in_format, VkImageLayout in_old, VkImageLayout in_new);
	void CopyBufferToImage(VkBuffer& in_buffer, VkImage& in_image, uint32_t in_width, uint32_t in_height);
	void CreateImageView(VkImageView& in_view, VkFormat in_format, VkImageAspectFlags in_aspectFlags, VkImage& in_image);

	// Shader functions
	std::vector<char> ReadFile(const std::string& in_fileName);
	VkShaderModule CreateShaderModule(const VkDevice in_device, const std::vector<char>& in_code);

	// Helper functions
	glm::vec3 NormalizeRGB(const glm::vec3& c);

	uint32_t FindMemoryType(uint32_t in_typeFilter, VkMemoryPropertyFlags in_properties);

	void CreateBuffer(VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkMemoryPropertyFlags in_props, VkBuffer& in_buffer, VkDeviceMemory& in_bufferMemory);
	void CopyBuffer(VkBuffer in_srcBuffer, VkBuffer in_dstBuffer, VkDeviceSize in_size);

private:
	// Instance member variables
	eVulkanInitState m_state = eVulkanInitState::Created;

	// Other member variables
	Mega::Renderer* m_pRenderer;
	Mega::Camera::ViewData m_viewData;

	// GLFW member variables
	GLFWwindow* m_pWindow;

	// ImGui
	ImguiObject m_imguiObject;

	VkDescriptorPool m_imguiDescriptorPool;
	ImGui_ImplVulkanH_Window m_imguiWindow;
	std::vector<ImGui_ImplVulkanH_Frame> m_imguiFramebuffers;

	// Vulkan member variables
	VkInstance m_instance = nullptr;
	VkSurfaceKHR m_surface = nullptr;

	VkPhysicalDevice m_physicalDevice = nullptr;
	VkPhysicalDeviceProperties m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures m_physicalDeviceFeatures;

	VkDevice m_device = nullptr;

	VkPipeline m_graphicsPipeline;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;
	VkShaderModule m_vertShaderModule;
	VkShaderModule m_fragShaderModule;

	VkRenderPass m_renderPass;
	VkRenderPass m_imguiRenderPass;

	VkSwapchainKHR m_swapchain;

	VkSurfaceFormatKHR m_surfaceFormat;
	VkPresentModeKHR m_presentMode;
	VkExtent2D m_swapchainExtent;

	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_swapchainImageViews;
	std::vector<VkFramebuffer> m_swapchainFramebuffers;

	std::vector<VkCommandPool> m_drawCommandPools;
	std::vector<VkCommandBuffer> m_drawCommandBuffers;

	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;

	std::vector<VkBuffer> m_uniformBuffersVert;
	std::vector<VkDeviceMemory> m_uniformBuffersMemoryVert;
	std::vector<VkBuffer> m_uniformBuffersFrag;
	std::vector<VkDeviceMemory> m_uniformBuffersMemoryFrag;

	size_t m_currentFrame = 0;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;

	QueueFamilyIndices m_queueFamilyIndices;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	std::vector<const char*> m_physicalDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME_MINE };

	// Z-Buffering
	ImageObject m_depthObject;

	// Vertices
	std::vector<Mega::Vertex> m_vertices;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory; // Handle to vertex buffer

	std::vector<uint32_t> m_indices;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	VkQueryPool m_queryPool;

	// Assets
	VkSampler m_sampler;

	std::vector<ImageObject> m_textures;
	uint32_t m_loadedTextureCount = 0;

	// Other
	VkBuffer m_stagingBuffer;
	VkDeviceMemory m_stagingBufferMemory;
};