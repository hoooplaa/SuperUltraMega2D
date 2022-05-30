#include "Vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyObjLoader/tiny_obj_loader.h>

#define VERTEX_BUFFER_SIZE 100000

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <array>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>

#include "VulkanImgui.h"
#include "VulkanDefines.h"

//////////////////// GET RID OF DEFAULT LOADING A BOX INTO VERTEX BUFFER BAD

class Model
{
public:
	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Mat3 = glm::mat3;
	using Mat4 = glm::mat4x4;

	struct PushConstant {
		Mat4 model = Mat4(1.0f);
		Vec4 color = Vec4(1.0f);

		Vec2 texCoordAdd = Vec2(0.0f, 0.0f);
		Vec2 texCoordMult = Vec2(1.0f, 1.0f);

		int32_t textureData = -1;
	};

public:
	void GetPushConstantData(Model::PushConstant* in_pData) const
	{

	}
};

// ================================ Public Functions ============================= //
void Vulkan::Initialize(Mega::Renderer* in_pRenderer, GLFWwindow* in_pWindow)
{
	std::cout << "\n=============== Initializing Vulkan ==============" << std::endl;

	// Store a pointer to the main renderer for data transfering and window for rendering/setup
	assert(in_pRenderer != nullptr && "ERROR: Cannot pass nullptr in place of Renderer* in Vulkan::Initialize()");
	assert(in_pWindow != nullptr && "ERROR: Cannot pass nullptr in place of Window* in Vulkan::Initialize()");
	m_pRenderer = in_pRenderer;
	m_pWindow = in_pWindow;

	// Initialize Vulkan
	CreateInstance(); // Create and store instance

	CreateWin32SurfaceKHR(m_pWindow, m_surface); // Create and store the surface
	PickPhysicalDevice(m_physicalDevice); // Pick and store suitable GPU to use

	CreateLogicalDevice(m_device); // Create and store the logical device

	CreateSwapchain(m_pWindow, m_surface, m_swapchain); // Create swapchain
	RetrieveSwapchainImages(m_swapchainImages, m_swapchain);
	CreateSwapchainImageViews(m_swapchainImageViews, m_swapchainImages); // Create image views for swapchain

	CreateRenderPass();
	CreateDepthResources(m_depthObject);

	CreateDrawCommandPools(m_drawCommandPools);

	CreateTextureSampler(m_sampler);

	CreateVertexBuffer(m_vertices, m_vertexBuffer, m_vertexBufferMemory);

	CreateDescriptorSetLayout(m_device, m_descriptorSetLayout);
	CreateGraphicsPipeline(m_vertShaderModule, m_fragShaderModule, m_graphicsPipeline);
	CreateFramebuffers(m_swapchainFramebuffers);

	CreateDrawCommands(m_drawCommandBuffers, m_drawCommandPools);

	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();

	CreateSyncObjects();

	m_imguiObject.Initialize(m_pWindow);
	m_imguiObject.CreateRenderData(this);

	// =================== OPTIMIZATION OCTOBER ============================ //
	// uint32_t queryCount = 5;
	// 
	// VkQueryPoolCreateInfo queryCreateInfo{};
	// queryCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	// queryCreateInfo.queryCount = queryCount;
	// 
	// vkCreateQueryPool(m_device, &queryCreateInfo, nullptr, &m_queryPool);
	// ===================================================================== //

	std::cout << "=============== Vulkan Inititalized ==============\n" << std::endl;
}

void Vulkan::Destroy()
{
	vkDeviceWaitIdle(m_device);

	// ============= ImGui ============= //
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(m_device, m_imguiDescriptorPool, nullptr);
	// ================================= //

	// Cleanup Vulkan
	CleanupSwapchain(&m_swapchain);

	vkDestroySampler(m_device, m_sampler, nullptr);
	for (auto& t : m_textures) { ImageObject::Destroy(&m_device, &t); }

	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);
	vkFreeMemory(m_device, m_indexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
	vkFreeMemory(m_device, m_vertexBufferMemory, nullptr);
	vkDestroyBuffer(m_device, m_stagingBuffer, nullptr);
	vkFreeMemory(m_device, m_stagingBufferMemory, nullptr);

	vkDestroyShaderModule(m_device, m_vertShaderModule, nullptr);
	vkDestroyShaderModule(m_device, m_fragShaderModule, nullptr);

	for (auto& pool : m_drawCommandPools) {
		vkDestroyCommandPool(m_device, pool, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
	}

	vkDestroyDevice(m_device, nullptr);
	vkDestroyInstance(m_instance, nullptr);

}
void Vulkan::CleanupSwapchain(VkSwapchainKHR* in_swapchain)
{
	ImageObject::Destroy(&m_device, &m_depthObject);

	for (size_t i = 0; i < m_swapchainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(m_device, m_swapchainFramebuffers[i], nullptr);
	}

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	for (size_t i = 0; i < m_swapchainImages.size(); i++) {
		vkDestroyBuffer(m_device, m_uniformBuffersVert[i], nullptr);
		vkFreeMemory(m_device, m_uniformBuffersMemoryVert[i], nullptr);
		vkDestroyBuffer(m_device, m_uniformBuffersFrag[i], nullptr);
		vkFreeMemory(m_device, m_uniformBuffersMemoryFrag[i], nullptr);
	}

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}
void Vulkan::RecreateSwapchain()
{
	std::cout << "Recreating swapchain..." << std::endl;

	//int width = 0, height = 0;
	//glfwGetFramebufferSize(m_pWindow, &width, &height);
	//while (width == 0 || height == 0) {
	//	glfwGetFramebufferSize(m_pWindow, &width, &height);
	//	glfwWaitEvents();
	//}

	vkDeviceWaitIdle(m_device);

	CleanupSwapchain(&m_swapchain);

	CreateSwapchain(m_pWindow, m_surface, m_swapchain);
	CreateSwapchainImageViews(m_swapchainImageViews, m_swapchainImages); // Create image views for swapchain
	CreateRenderPass();
	CreateGraphicsPipeline(m_vertShaderModule, m_fragShaderModule, m_graphicsPipeline);
	CreateDepthResources(m_depthObject);
	CreateFramebuffers(m_swapchainFramebuffers);
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateDrawCommands(m_drawCommandBuffers, m_drawCommandPools);

	// ImGui //
	//ImGui_ImplVulkan_SetMinImageCount(2);
	//ImGui_ImplVulkanH_CreateOrResizeWindow(m_instance, m_physicalDevice, m_device, &m_imguiWindow,
		//static_cast<uint32_t>(m_queueFamilyIndices.graphicsFamily.value()), VK_NULL_HANDLE, 500, 500, uint32_t(1));
}

void Vulkan::DrawFrame(std::vector<Mega::Batch>& in_batches)
{
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			glfwWaitEvents();
		}
		//RecreateSwapchain();
	}
	else {
		assert((result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) && "ERROR: Failed to acquire swapchain image");
	}

	// Check if a previous frame is using this image (i.e. there is its fence to wait on) /////////////////// FIX THIS SHIT hella slow ///////////////////////////
	if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// =================================== //

	// Mark the image as now being in use by this frame
	m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	UpdateUniformBuffer(imageIndex);

	// ======================= Draw Shit =============== //

	auto* commandBuffer = &m_drawCommandBuffers[imageIndex];
	// auto* commandBuffer = &m_imguiObject.m_frames[imageIndex].CommandBuffer;

	// Multiple colors for the depth and image attachment
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { CLEAR_COLOR } };
	if (DEPTH_BUFFERING) { clearValues[1].depthStencil = { 1.0f, 0 }; }
	else { clearValues[1].depthStencil = { 1.0f }; }

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	result = vkBeginCommandBuffer(*commandBuffer, &beginInfo);
	assert(result == VK_SUCCESS && "vkBeginCommandBuffer() did not return success");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex]; // Remember, framebuffers reference VkImageView objects which represent the attachments 
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapchainExtent;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(*commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[imageIndex], 0, nullptr);

	// ==================== Draw ================== //

	// Update vertex data with batches
	UpdateVertexData(in_batches);
	VkBuffer vertexBuffers1[] = { m_vertexBuffer };

	VkDeviceSize offsets1[] = { 0 };

	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers1, offsets1);
	// vkCmdBindIndexBuffer(*commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

	for (const auto& batch : in_batches)
	{
		Model::PushConstant p;
		p.textureData = batch.texture.index;
		vkCmdPushConstants(*commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Model::PushConstant), &p);

		vkCmdDraw(*commandBuffer, batch.vertices.size(), 1, batch.vertexIndices[0], 0);
	}


	// ======================= ImGui =================== //

	if (ImGui::GetDrawData())
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer, NULL);
	}

	// ================================================= //

	vkCmdEndRenderPass(*commandBuffer);

	result = vkEndCommandBuffer(*commandBuffer);
	assert(result == VK_SUCCESS && "ERROR: vkEndCommandBuffer() did not return success");

	// ================================================= //

	std::vector<VkCommandBuffer> submitCommands = {
		*commandBuffer,
	};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = submitCommands.data();

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);
	assert(result == VK_SUCCESS && "ERROR: vkQueueSubmit() did not return succes");

	VkSwapchainKHR swapChains[] = { m_swapchain };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(m_pWindow, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(m_pWindow, &width, &height);
			glfwWaitEvents();
		}
		//RecreateSwapchain();
	}
	else {
		assert((result == VK_SUCCESS) && "ERROR: Failed to acquire swapchain image");
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
};

void Vulkan::SetViewData(const Mega::Camera::ViewData& in_viewData) {
	m_viewData = in_viewData;
}
void Vulkan::CreateTextureImage(ImageObject& in_imageObject, const char* in_filename)
{
	// "Create a buffer in host visible memory so that we can use vkMapMemory and copy the pixels to it"

	int width, height, texChannels;
	stbi_uc* pixels = stbi_load(in_filename, &width, &height, &texChannels, STBI_rgb_alpha);

	if (!pixels) {
		std::cout << "Failed to load Texture at " << in_filename << std::endl;
		throw std::runtime_error("ERROR: Failed to load texture image!");
		assert(false && "Prob wrong texture path or not there");
	}

	VkDeviceSize imageSize = width * height * 4;
	in_imageObject.extent = glm::vec2(width, height);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Copy the pixel data into the buffer
	void* data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);

	// Create the VkImage object
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	CreateImageObject(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, in_imageObject.image, in_imageObject.memory);

	// Copy the buffer data to the image object, after transitioning the layout, then transition it again for optimal shader reading
	ChangeImageLayout(in_imageObject.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(stagingBuffer, in_imageObject.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	ChangeImageLayout(in_imageObject.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Cleanup
	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}
void Vulkan::LoadTextureData(const char* in_texPath, Mega::Texture* in_pTextureData) {
	uint32_t index = m_loadedTextureCount;

	std::cout << "Texture number " << index << " loaded" << std::endl;

	if (m_loadedTextureCount > MAX_TEXTURE_COUNT) {
		throw std::exception("ERROR: Max texture limit reached");

		while (index >= MAX_TEXTURE_COUNT) { index = m_loadedTextureCount % MAX_TEXTURE_COUNT; }
	}
	if (m_loadedTextureCount >= m_textures.size()) {
		ImageObject image;
		m_textures.push_back(image);
	}

	//  Create
	CreateTextureImage(m_textures[index], in_texPath);
	CreateImageView(m_textures[index].view, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_textures[index].image);
	++m_loadedTextureCount;

	// Update
	// UpdateDescriptorSets();

	// Output
	in_pTextureData->index = index;
	in_pTextureData->width = m_textures[index].extent.x;
	in_pTextureData->height = m_textures[index].extent.y;
}

// ================================ Private Functions ============================= //

void Vulkan::CreateInstance()
{
	std::cout << "Creating Vulkan Instance" << std::endl;

	bool result1 = CheckValidationLayerSupport();
	bool result2 = CheckGLFWExtensionSupport();
	assert(result1 && "ERROR: A validation layer is not available");
	assert(result2 && "ERROR: Not all of GLFW's required extensions are supported");

	VkApplicationInfo appInfo{}; // Zeroing the struct to make sure all values are set to 0 or null
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Have to specify the type of struct
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// "Vulkan is a platform agnostic API, which means that you need an extension to interface with the window system"
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo{}; // Another struct to "tell the Vulkan driver which global extensions and validation layers we want to use"
	if (g_enableValidationLayers) {
		createInfo.enabledLayerCount = 0; // static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // Again, we have to specify the type of struct
	createInfo.pApplicationInfo = &appInfo; // Adding the app data to the create struct
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
	assert(result == VK_SUCCESS && "ERROR: Vulkan vkCreateInstance() did not return success");
}
bool Vulkan::CheckValidationLayerSupport()
{
	// Checks if all requested validation layers are available

	uint32_t vk_layerCount = 0;
	vkEnumerateInstanceLayerProperties(&vk_layerCount, nullptr); // First enumerate to get the number of layers

	std::vector<VkLayerProperties> vk_availableLayers(vk_layerCount);
	vkEnumerateInstanceLayerProperties(&vk_layerCount, vk_availableLayers.data()); // Enumerate again to add layers to vk_availableLayers

	// Check to make sure every Layer we are adding is in vk_availableLayers
	std::vector<const char*> unfoundLayerNames;
	for (auto& addedLayer : m_validationLayers) { // For each validation layer in m_validationLayers;
		bool layerFound = false;

		for (auto& vk_layer : vk_availableLayers) { // Check if its in vk_availableLayers
			if (strcmp(vk_layer.layerName, addedLayer)) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) { // Add it to our list of unfound layers
			unfoundLayerNames.push_back(addedLayer);
		}
	}

	// Cout our data
	std::cout << "Vulkan Validation Layers:\n";
	for (auto& layer : vk_availableLayers) {
		std::cout << layer.layerName << ": Version " << layer.specVersion << "\n";
	}

	if (unfoundLayerNames.size() > 0) {
		std::cout << "Unfound Validation Layers\n";
		for (auto& layer : unfoundLayerNames) {
			std::cout << layer << " not found" << "\n";
		}
		return false;
	}
	else {
		std::cout << "All required validation layers found\n";
		return true;
	}
}
bool Vulkan::CheckGLFWExtensionSupport()
{
	// Checks if all of GLFW's required extensions are supported

	uint32_t vk_extensionCount = 0;
	uint32_t glfw_extensionCount = 0;
	const char** glfw_requiredExtensions;

	vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensionCount, nullptr); // First enumerate to get the number of extensions
	std::vector<VkExtensionProperties> vk_availableExtensions(vk_extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &vk_extensionCount, vk_availableExtensions.data()); // Enumerate again to add extensions to vk_availableExtensions

	glfw_requiredExtensions = glfwGetRequiredInstanceExtensions(&glfw_extensionCount);

	// Check to make sure every Extension in glfwRequiredInstanceExtensions() is in our extension list
	std::vector<const char*> glfw_unfoundExtensions;
	for (uint32_t i = 0; i < glfw_extensionCount; i++) { // For each reuired GLFW extension...
		bool extensionFound = false;

		for (auto& vk_extension : vk_availableExtensions) { // Check if its in Vulkan's extension list
			if (strcmp(vk_extension.extensionName, *glfw_requiredExtensions)) {
				extensionFound = true;
				break;
			}
		}

		if (!extensionFound) { // Add it to our list of unfound required GLFW extensions
			glfw_unfoundExtensions.push_back(*glfw_requiredExtensions);
		}

		++glfw_requiredExtensions;
	}

	// Cout our data
	std::cout << "Found Vulkan Extensions:\n";
	for (auto& extension : vk_availableExtensions) {
		std::cout << extension.extensionName << ": Version " << extension.specVersion << "\n";
	}

	if (glfw_unfoundExtensions.size() > 0) {
		std::cout << "Unfound Required GLFW Extensions\n";
		for (auto& extension : glfw_unfoundExtensions) {
			std::cout << extension << " not found" << "\n";
		}
		return false;
	}
	else {
		std::cout << "All required GLFW extensions found\n";
		return true;
	}
}

void Vulkan::CreateWin32SurfaceKHR(GLFWwindow* in_pWindow, VkSurfaceKHR& in_surface)
{
	std::cout << "Creating surface..." << std::endl;

	assert(in_pWindow != nullptr && "ERROR: Cannot create surface with null GLFW window");
	assert(m_instance != nullptr && "ERROR: Cannot create surface without a Vulkan instance");

	//int counter = 0;
	//HWND target = nullptr;
	//
	//for (HWND hwnd = GetTopWindow(NULL); hwnd != NULL; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT))
	//{
	//
	//	if (!IsWindowVisible(hwnd)) { continue; }
	//
	//	const int length = GetWindowTextLength(hwnd);
	//	if (length == 0) { continue; }
	//
	//	wchar_t* title = new wchar_t[length + 1];
	//	GetWindowText(hwnd, title, length + 1);
	//
	//	++counter;
	//	std::cout << counter << ": ";
	//	std::wcout << "HWND: " << hwnd << " Title: " << title << std::endl;
	//
	//	if (counter == 6) { target = hwnd; }
	//}

	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(in_pWindow);
	//createInfo.hwnd = target;
	createInfo.hinstance = GetModuleHandle(nullptr);

	VkResult result = vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &in_surface);
	assert(result == VK_SUCCESS && "ERROR: vkCreateWin32SurfaceKHR() did not return success");
}

void Vulkan::PickPhysicalDevice(VkPhysicalDevice& in_device)
{
	// Picks a suitable GPU to use

	std::cout << "Picking physical device..." << std::endl;

	assert(m_instance != nullptr && "ERROR: Cannot pick physical device using nullptr instance");
	assert(m_surface != nullptr && "ERROR: cannot pick physical device without a surface");

	uint32_t deviceCount = 0; // Doing the same thing where we enumerate each device and store the data
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	assert(deviceCount > 0 && "ERROR: Could not find a GPU with Vulkan Support");

	for (auto& device : devices) { // Go through each device we found and pick the best one
		std::cout << "Device: " << device << std::endl;
		std::cout << "Surface: " << m_surface << std::endl;
		if (IsPhysicalDeviceSuitable(device, m_surface)) { // For now, just pick the best one
			in_device = device;
			break;
		}
	}

	assert(in_device != VK_NULL_HANDLE && "ERROR: Could not find a suitable GPU");

	m_queueFamilyIndices = Vulkan::FindQueueFamilies(in_device, m_surface); // Fill in the queue family indices
	vkGetPhysicalDeviceProperties(in_device, &m_physicalDeviceProperties); // Fill in the properties and features for later use
	vkGetPhysicalDeviceFeatures(in_device, &m_physicalDeviceFeatures);
}
QueueFamilyIndices Vulkan::FindQueueFamilies(const VkPhysicalDevice in_device, VkSurfaceKHR in_surface)
{
	// Makes a QueueFamilyIndices struct and iterates through each queue family of
	// in_physicalDevice and fills in the struct with the indice of that queue family

	QueueFamilyIndices out_indices;

	uint32_t queueFamilyCount = 0; // Doing a very similar thing to enumerate through the list of queue families
	vkGetPhysicalDeviceQueueFamilyProperties(in_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(in_device, &queueFamilyCount, queueFamilies.data());

	int i = 0; // Pick a queue family that supports	VK_QUEUE_GRAPHICm_BIT
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false; // If the queue family can support drawing to in_surface
		vkGetPhysicalDeviceSurfaceSupportKHR(in_device, i, in_surface, &presentSupport);

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			out_indices.graphicsFamily = i;
		}
		if (presentSupport) { // Both if and not else if because drawing and presenting are probably done in the same queue family
			out_indices.presentFamily = i;
		}

		++i;
	}

	return out_indices;
}
bool Vulkan::IsPhysicalDeviceSuitable(const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface)
{
	// Tests if the physical device is suitable for our needs (specifically has graphics queue and can
	// draw to our surface, or finding a queue family that supports presenting to the surface we created)
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(in_device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(in_device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(in_device, m_surface);
	bool extensionsSupported = CheckPhysicalDeviceExtensionSupport(in_device);

	// Test swap chain support of device
	bool isSwapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(in_device, in_surface);
		isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported && isSwapChainAdequate && deviceFeatures.samplerAnisotropy;
}
bool Vulkan::CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice in_device)
{
	// Checks if the physical device supports all of the extensions we want to use in m_physicalDeviceExtensions

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(in_device, nullptr, &extensionCount, nullptr); // Enumerate through the physical device extensions in a similar way as the other Check functions

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(in_device, nullptr, &extensionCount, availableExtensions.data());

	std::unordered_set<std::string> requiredExtensions(m_physicalDeviceExtensions.begin(), m_physicalDeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}
SwapChainSupportDetails Vulkan::QuerySwapChainSupport(const VkPhysicalDevice in_device, const VkSurfaceKHR in_surface)
{
	// Populates a SwapChainSupportDetails struct and makes sure its compatable with our surface

	SwapChainSupportDetails out_swapChainDetails{};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(in_device, in_surface, &out_swapChainDetails.surfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(in_device, in_surface, &formatCount, nullptr);

	out_swapChainDetails.formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(in_device, in_surface, &formatCount, out_swapChainDetails.formats.data());

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(in_device, in_surface, &presentModeCount, nullptr);

	out_swapChainDetails.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(in_device, in_surface, &presentModeCount, out_swapChainDetails.presentModes.data());

	return out_swapChainDetails;
}

void Vulkan::CreateLogicalDevice(VkDevice& in_device)
{
	std::cout << "Creating logical device..." << std::endl;

	assert(m_physicalDevice != nullptr && "ERROR: Cannot create a Vulkan logical device without a physical device");

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos; // A list of VkDeviceQueueCreateInfo for each queue in m_queueFamilyIndices

	// Making sure both queueFamilyIndices have values before adding them
	std::set<uint32_t> uniqueQueueFamilies = { m_queueFamilyIndices.graphicsFamily.value(), m_queueFamilyIndices.presentFamily.value() };
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};

		float queuePriority = 1.0f;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_physicalDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = m_physicalDeviceExtensions.data();

	if (g_enableValidationLayers) { // Should this be here?
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &in_device);
	assert(result == VK_SUCCESS && "ERROR: Vulkan vkCreateDevice() did not return a success");

	std::cout << "Logical device: " << in_device << std::endl;

	vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamily.value(), 0, &m_graphicsQueue); // Store the graphics queue of the device
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamily.value(), 0, &m_presentQueue); // Store the present queue of the device
}

void Vulkan::CreateSwapchain(GLFWwindow* in_pWindow, const VkSurfaceKHR in_surface, VkSwapchainKHR& in_swapchain)
{
	std::cout << "Creating Swapchain..." << std::endl;

	assert(m_physicalDevice != nullptr && "ERROR: Cannot create a swapchain with a null physical device");
	assert(m_device != nullptr && "ERROR: Cannot create a swapchain with a null logical device");

	assert(in_surface != nullptr && "ERROR: Cannot create a swapchain with a null surface");

	// Query the details of our swapchain given our chosen physical device and surface, because not eveything will be compatable with the surface etc
	SwapChainSupportDetails swapSupportDetails = Vulkan::QuerySwapChainSupport(m_physicalDevice, m_surface);

	// Choose the best ones out of our options
	std::cout << "SwapSupportDetails.formats size: " << swapSupportDetails.formats.size() << std::endl;
	m_surfaceFormat = Vulkan::ChooseSwapSurfaceFormat(swapSupportDetails.formats);
	m_presentMode = Vulkan::ChooseSwapPresentMode(swapSupportDetails.presentModes);
	m_swapchainExtent = Vulkan::ChooseSwapExtent(swapSupportDetails.surfaceCapabilities, in_pWindow); // Store the swapchain extent for later use

	uint32_t imageQueueCount = swapSupportDetails.surfaceCapabilities.minImageCount + 1; // Set number of images in queue
	uint32_t maxImages = swapSupportDetails.surfaceCapabilities.maxImageCount; // Cap it to the surface's maximum
	if (maxImages > 0 && imageQueueCount > maxImages) { imageQueueCount = maxImages; }

	VkSwapchainCreateInfoKHR createInfo{}; // Create info for swap chain
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = in_surface;
	createInfo.minImageCount = imageQueueCount;
	createInfo.imageFormat = m_surfaceFormat.format;
	createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
	createInfo.imageExtent = m_swapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, in_surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // Image is owned by one queue family at a time and ownership must be explicitly
																 //  transferred before using it in another queue family, offers best performance
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapSupportDetails.surfaceCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = m_presentMode;
	createInfo.clipped = VK_TRUE; // Dont care about pixels that are obscured, like another window in front of it
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &in_swapchain);
	assert(result == VK_SUCCESS && "ERROR: vkCreateSwapchainKHR() did not return success");
}
VkSurfaceFormatKHR Vulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& in_availableFormats)
{
	// Pick what surface format we want to use (color depth)

	assert(!in_availableFormats.empty() && "ERROR: Trying to chose a surface format from a vector of 0 choices in ChooseSwapSurfaceFormat()");

	for (const auto& availableFormat : in_availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return in_availableFormats[0]; // Eventually make function that chooses best one
}
VkPresentModeKHR Vulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& in_availableModes)
{
	// Choose the conditions for showing images to the screen (probably most important)

	assert(!in_availableModes.empty() && "Trying to choose a present mode from a vector of 0 choices in ChooseSwapPresentMode()");

	for (const auto& presentMode : in_availableModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // Prefer VK_PRESENT_MODE_MAILBOX_KHR
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available
}
VkExtent2D Vulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& in_capabilities, GLFWwindow* in_pWindow)
{
	// Choose VkExtent2D (resoluion of the swap chain images)
	// currentExtent (width/height of surface) is 0xFFFFFFFF if the "surface size [is] determined by the extent of a swapchain targeting the surface" 
	if (in_capabilities.currentExtent.width != UINT32_MAX) { return in_capabilities.currentExtent; }

	int width, height;
	glfwGetFramebufferSize(in_pWindow, &width, &height);

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	actualExtent.width = std::clamp(actualExtent.width, in_capabilities.minImageExtent.width, in_capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, in_capabilities.minImageExtent.height, in_capabilities.maxImageExtent.height);

	return actualExtent;
}

void Vulkan::RetrieveSwapchainImages(std::vector<VkImage>& in_images, VkSwapchainKHR in_swapchain)
{
	// Get a handle to the swapchain images in the form of VkImages

	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_device, in_swapchain, &imageCount, nullptr);

	in_images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, in_swapchain, &imageCount, in_images.data());
}
void Vulkan::CreateSwapchainImageViews(std::vector<VkImageView>& in_imageViews, const std::vector<VkImage>& in_images)
{
	// To use a VkImage, we have to create a VkImageView for it, which is just a 'view' to that image
	// It describes how to access the image and which part of the image to access
	// "An image view is sufficient to start using an image as a texture, but it's not quite ready to be
	// used as a render target just yet. That requires one more step of indirection, known as a framebuffer"
	// This function creates and returns an ImageView for each image in the swapchain

	in_imageViews.resize(in_images.size());

	for (size_t i = 0; i < in_images.size(); i++) {
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = in_images[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = m_surfaceFormat.format;

		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Equivalent to VK_COMPONENT_SWIZZLE_R
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // and so on
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(m_device, &viewCreateInfo, nullptr, &in_imageViews[i]);
		assert(result == VK_SUCCESS && "ERROR: vkCreateImageView() did not return sucess");
	}
}

void Vulkan::CreateDescriptorPool()
{
	std::cout << "Creating Descriptor Pool..." << std::endl;

	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size());
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(m_swapchainImages.size()) * MAX_TEXTURE_COUNT;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_swapchainImages.size());

	VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool);
	assert(result == VK_SUCCESS && "ERROR: vkCreateDescriptorPool() did not return success");
}
void Vulkan::CreateDescriptorSetLayout(const VkDevice in_device, VkDescriptorSetLayout& in_descriptorSetLayout)
{
	VkDescriptorSetLayoutBinding uboLayoutBindingVert{};
	uboLayoutBindingVert.binding = 0;
	uboLayoutBindingVert.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingVert.descriptorCount = 1;
	uboLayoutBindingVert.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding uboLayoutBindingFrag{};
	uboLayoutBindingFrag.binding = 1;
	uboLayoutBindingFrag.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBindingFrag.descriptorCount = 1;
	uboLayoutBindingFrag.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 2;
	samplerLayoutBinding.descriptorCount = MAX_TEXTURE_COUNT;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBindingVert, uboLayoutBindingFrag, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(in_device, &layoutInfo, nullptr, &in_descriptorSetLayout);
	assert(result == VK_SUCCESS && "ERROR: vkCreateDescriptorSetLayout() did not return success");
}
void Vulkan::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_swapchainImageViews.size(), m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapchainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(m_swapchainImages.size());
	VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data());
	assert(result == VK_SUCCESS && "vkAllocateDescriptorSets() did not return success");

	UpdateDescriptorSets();
}
void Vulkan::UpdateDescriptorSets()
{
	for (size_t i = 0; i < m_swapchainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfoVert{};
		bufferInfoVert.buffer = m_uniformBuffersVert[i];
		bufferInfoVert.offset = 0;
		bufferInfoVert.range = sizeof(UniformBufferObjectVert);

		VkDescriptorBufferInfo bufferInfoFrag{};
		bufferInfoFrag.buffer = m_uniformBuffersFrag[i];
		bufferInfoFrag.offset = 0;
		bufferInfoFrag.range = sizeof(UniformBufferObjectFrag);

		std::vector<VkDescriptorImageInfo> imageInfo(m_textures.size());
		for (int i = 0; i < m_textures.size(); ++i) {
			imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo[i].imageView = m_textures[i].view;
			imageInfo[i].sampler = m_sampler;
		}

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfoVert;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &bufferInfoFrag;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = m_descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = m_textures.size();
		descriptorWrites[2].pImageInfo = imageInfo.data();

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Vulkan::CreateVertexBuffer(std::vector<Mega::Vertex>& in_vertices, VkBuffer& in_buffer, VkDeviceMemory& in_memory)
{
	// Creates a Vulkan vertex buffer that we can use to pass the vertex data to our gpu and fills it with the Renderer's vertex data

	std::cout << "Creating vertex buffer..." << std::endl;

	in_vertices.resize(VERTEX_BUFFER_SIZE);

	VkDeviceSize bufferSize = sizeof(in_vertices[0]) * in_vertices.size();

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_stagingBuffer, m_stagingBufferMemory);

	// Map the veretx data to the buffer

	void* data;
	vkMapMemory(m_device, m_stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, in_vertices.data(), sizeof(in_vertices));
	vkUnmapMemory(m_device, m_stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, in_buffer, in_memory);

	CopyBuffer(m_stagingBuffer, in_buffer, bufferSize);
}

void Vulkan::UpdateVertexData(std::vector<Mega::Batch>& in_batches)
{
	// For each batch, add the vertex data to the vertex buffer and add correct indexes so we can draw the batches later
	m_vertices.clear();
	for (auto& batch : in_batches)
	{
		batch.vertexIndices[0] = m_vertices.size();

		m_vertices.insert(m_vertices.end(), batch.vertices.begin(), batch.vertices.end());

		batch.vertexIndices[1] = m_vertices.size();
	}

	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	assert(sizeof(m_vertices[0]) * m_vertices.size() <= VERTEX_BUFFER_SIZE && "ERROR: Vertex buffer full");

	// Map the veretx data to the buffer
	void* data;
	vkMapMemory(m_device, m_stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, m_stagingBufferMemory);

	CopyBuffer(m_stagingBuffer, m_vertexBuffer, bufferSize);
}
void Vulkan::UpdateLoadedTextureData()
{
	UpdateDescriptorSets();
}

void Vulkan::CreateUniformBuffers()
{
	std::cout << "Creating Uniform Buffers..." << std::endl;

	VkDeviceSize bufferSizeVert = sizeof(UniformBufferObjectVert);
	VkDeviceSize bufferSizeFrag = sizeof(UniformBufferObjectFrag);

	m_uniformBuffersVert.resize(m_swapchainImages.size());
	m_uniformBuffersMemoryVert.resize(m_swapchainImages.size());

	m_uniformBuffersFrag.resize(m_swapchainImages.size());
	m_uniformBuffersMemoryFrag.resize(m_swapchainImages.size());

	for (size_t i = 0; i < m_swapchainImages.size(); i++) {
		CreateBuffer(bufferSizeVert, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffersVert[i], m_uniformBuffersMemoryVert[i]);

		CreateBuffer(bufferSizeFrag, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffersFrag[i], m_uniformBuffersMemoryFrag[i]);
	}
}
void Vulkan::UpdateUniformBuffer(uint32_t in_imageIndex)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// Vertex UBO
	UniformBufferObjectVert uboVert{};
	uboVert.view = glm::lookAt(m_viewData.eye, m_viewData.target, m_viewData.up);
	uboVert.proj = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 1000.0f);
	uboVert.proj[1][1] *= -1; // Flipping the Y coordinates because opengl uses inverted y coordinates

	void* dataVert;
	vkMapMemory(m_device, m_uniformBuffersMemoryVert[in_imageIndex], 0, sizeof(uboVert), 0, &dataVert);
	memcpy(dataVert, &uboVert, sizeof(uboVert));
	vkUnmapMemory(m_device, m_uniformBuffersMemoryVert[in_imageIndex]);

	// Fragment UBO
	UniformBufferObjectFrag uboFrag{};

	uboFrag.viewDir = m_viewData.target - m_viewData.eye;
	uboFrag.viewPos = m_viewData.eye;

	uboFrag.lightCount = 0; // in_pLights.size();
	//for (int i = 0; i < in_pLights.size(); ++i)
	//{
	//	uboFrag.lights[i] = *in_pLights[i];
	//}

	void* dataFrag;
	vkMapMemory(m_device, m_uniformBuffersMemoryFrag[in_imageIndex], 0, VK_WHOLE_SIZE, 0, &dataFrag);
	memcpy(dataFrag, &uboFrag, sizeof(uboFrag));
	vkUnmapMemory(m_device, m_uniformBuffersMemoryFrag[in_imageIndex]);
}

void Vulkan::CreateGraphicsPipeline(VkShaderModule& in_vertShaderModule, VkShaderModule& in_fragShaderModule, VkPipeline& in_pipeline)
{
	std::cout << "Creating graphics pipeline..." << std::endl;

	assert(m_device != nullptr && "Cannot create a graphics pipeline with a null logical device");

	auto vertShaderCode = ReadFile(SHADER_PATH_VERT); // Can check if loaded in correctly if size of vertShaderCode == size of file
	auto fragShaderCode = ReadFile(SHADER_PATH_FRAG);

	//////////////////////////// Runtim Comp Shaders /////////////////////////////////////////
	//auto fragGLSLCode = ReadFile("Shaders/Shader.frag");
	//
	//std::vector<uint32_t> spirvCode;
	//
	//SpirvHelper::Initialize();
	//SpirvHelper::GLSLtoSPV(EShLanguage::EShLangFragment, fragGLSLCode.data(), spirvCode);
	//SpirvHelper::Destroy();
	//
	////////////////////////////////////////////////////////////////////////////////////////////

	in_vertShaderModule = CreateShaderModule(m_device, vertShaderCode);
	in_fragShaderModule = CreateShaderModule(m_device, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = in_vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = in_fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// Old version
	//vertexInputInfo.vertexBindingDescriptionCount = 0;
	//vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	//vertexInputInfo.vertexAttributeDescriptionCount = 0;
	//vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	// Now we want ot be able to accept data from vertex buffers and pass it to our shaders
	auto bindingDescription = Mega::Vertex::GetBindingDescription();
	auto attributeDescriptions = Mega::Vertex::GetAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; // Basically how Vulkan will draw the vertices we give it
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkViewport viewport{}; // What portion of the framebuffer we want to render to
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_swapchainExtent.width;
	viewport.height = (float)m_swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{}; // What part of the image we want to render
	scissor.offset = { 0, 0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{}; // Performs depth testing, face cullingand the scissor test
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = CULL_MODE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH,
	};

	VkPipelineDynamicStateCreateInfo dynamicState{}; // Stuff that can be changed without recreating the pipeline
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(Model::PushConstant);
	std::cout << "Push Constant Size: " << sizeof(Model::PushConstant) << std::endl;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
	assert(result == VK_SUCCESS && "ERROR: vkCreatePipelineLayout() did not return success");

	VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo }; // Might cause read access violation error

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	if (DEPTH_BUFFERING) { pipelineInfo.pDepthStencilState = &depthStencil; }

	result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &in_pipeline);
	assert(result == VK_SUCCESS && "ERROR: vkCreateGraphicsPipelines() did not return sucess");
}
void Vulkan::CreateRenderPass()
{
	// Set up to expect a framebuffer, which is used as render target

	std::cout << "Creating render pass..." << std::endl;

	assert(m_device != nullptr && "Cannot create a render pass with a null logical device");

	// "The attachments specified during render pass creation are bound by wrapping them into a VkFramebuffer object.
	// A framebuffer object references all of the VkImageView objects that represent the attachments.
	// In our case that will be only a single one : the color attachment."
	// In other words, a framebuffer object refernces the corresponding VkImageView object that represents the attachments
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	if (DEPTH_BUFFERING) { subpass.pDepthStencilAttachment = &depthAttachmentRef; }

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkAttachmentDescription> attachments;
	if (DEPTH_BUFFERING) { attachments = { colorAttachment, depthAttachment }; }
	else { attachments = { colorAttachment }; }

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
	assert(result == VK_SUCCESS && "ERROR: vkCreateRenderPass() did not return success");
}
void Vulkan::CreateFramebuffers(std::vector<VkFramebuffer>& in_swapchainFramebuffers)
{
	// Framebuffers are used as the render target in the render pass

	std::cout << "Creating framebuffers..." << std::endl;

	assert(m_device != nullptr && "Cannot create framebuffers with a null logical device");

	in_swapchainFramebuffers.resize(m_swapchainImageViews.size());

	for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
		std::vector<VkImageView> attachments;
		if (DEPTH_BUFFERING) { attachments = { m_swapchainImageViews[i], m_depthObject.view }; }
		else { attachments = { m_swapchainImageViews[i] }; }

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &in_swapchainFramebuffers[i]);
		assert(result == VK_SUCCESS && "ERROR: vkCreateFramebuffer() did not return success");
	}
}

void Vulkan::CreateDrawCommands(std::vector<VkCommandBuffer>& in_buffers, std::vector<VkCommandPool>& in_pools)
{
	// Creates a command buffer for each image in the swapchain

	std::cout << "Creating draw commands..." << std::endl;

	assert(m_device != nullptr && "ERROR: Cannot create a commands without a logical device");
	assert(m_renderPass != nullptr && "ERROR: Cannot create a commands without a render pass");

	in_buffers.resize(m_swapchainFramebuffers.size());

	for (int i = 0; i < m_swapchainFramebuffers.size(); ++i) {
		assert(in_pools[i] != nullptr && "ERROR: Cannot create a command without a command pool");

		AllocateCommandBuffer(in_buffers[i], in_pools[i]);
		//std::vector<Model> emptyModels;
		//RecordDrawCommandBuffer(emptyModels, m_swapchainFramebuffers[i], m_descriptorSets[i], in_buffers[i]);
	}

}
void Vulkan::CreateDrawCommandPools(std::vector<VkCommandPool>& in_pools)
{
	// Creates a command pool for each frame in the swapchain

	std::cout << "Creating draw command pools..." << std::endl;

	assert(m_device != nullptr && "ERROR: Cannot create a command pool using a null logical device");

	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_surface);

	in_pools.resize(m_swapchainImageViews.size());

	for (int i = 0; i < in_pools.size(); ++i) {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

		VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &in_pools[i]);
		assert(result == VK_SUCCESS && "ERROR: vkCreateCommanPool() did not return success");
	}
}
void Vulkan::AllocateCommandBuffer(VkCommandBuffer& in_buffer, const VkCommandPool& in_pool)
{
	// Allocates a command buffer into the command pool

	assert(m_device != nullptr && "ERROR: Cannot create a command buffer without a logical device");
	assert(in_pool != nullptr && "ERROR: Cannot allocate a command buffer without a command pool");

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = in_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = uint32_t(1);

	VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &in_buffer);
	assert(result == VK_SUCCESS && "ERROR: vkAllocateout_commandBuffers() did not return success");
}

VkCommandBuffer Vulkan::BeginSingleTimeCommand(const VkCommandPool& in_pool)
{
	// Creates, begins and returns a single time command buffer in the given command pool
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = in_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer out_command;
	vkAllocateCommandBuffers(m_device, &allocInfo, &out_command);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(out_command, &beginInfo);

	return out_command;
}
void Vulkan::EndSingleTimeCommand(const VkCommandPool& in_pool, VkCommandBuffer in_command)
{
	// Ends, submits into the graphics queue, and frees the given command buffer

	vkEndCommandBuffer(in_command);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &in_command;

	VkResult result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	assert(result == VK_SUCCESS && "ERROR: vkQueueSubmit() did not return success");

	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, in_pool, 1, &in_command);
}

void Vulkan::CreateDepthResources(ImageObject& in_depthObject)
{
	VkFormat depthFormat = FindDepthFormat();

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(m_swapchainExtent.width);
	imageInfo.extent.height = static_cast<uint32_t>(m_swapchainExtent.height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = depthFormat;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	CreateImageObject(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, in_depthObject.image, in_depthObject.memory);
	CreateImageView(in_depthObject.view, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, in_depthObject.image);
}
VkFormat Vulkan::FindSupportedFormat(const std::vector<VkFormat>& in_candidates, VkImageTiling in_tiling, VkFormatFeatureFlags in_features)
{
	// Some helpers to find if the supported image format is available for the depth image
	for (VkFormat format : in_candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

		if (in_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & in_features) == in_features) {
			return format;
		}
		else if (in_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & in_features) == in_features) {
			return format;
		}
	}

	std::runtime_error("ERROR: Could not find a supported format");

	return VkFormat(-1);
}
VkFormat Vulkan::FindDepthFormat()
{
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}
bool Vulkan::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Vulkan::CreateSyncObjects() // Create the semaphores and fences
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	m_imagesInFlight.resize(m_swapchainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkResult result1 = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
		VkResult result2 = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
		VkResult result3 = vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]);

		assert(result1 == VK_SUCCESS && result2 == VK_SUCCESS && result3 == VK_SUCCESS &&
			"ERROR: vkCreateSemaphore() or vkCreateFence() did not return success");
	}
}

// ================================ Other Functions ============================= //

void Vulkan::CreateTextureSampler(VkSampler in_sampler)
{
	// "Textures are usually accessed through samplers, which will apply filtering and transformations to compute the final color that is retrieved."
	// for example, bilinear filtering, or anisotropic filtering

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//samplerInfo.magFilter = VK_FILTER_LINEAR;
	//samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // Repeat the texture when going beyond the image dimensions.
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	// samplerInfo.anisotropyEnable = VK_TRUE;
	// samplerInfo.maxAnisotropy = m_physicalDeviceProperties.limits.maxSamplerAnisotropy;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = m_physicalDeviceProperties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkResult result = vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler);
	assert(result == VK_SUCCESS && "ERROR: vkCreateSampler() did not return success");
}

void Vulkan::CreateImageObject(VkImageCreateInfo& in_info, VkMemoryPropertyFlags in_properties, VkImage& in_image, VkDeviceMemory& in_imageMemory)
{
	// Creates and vkImage using the given info and passed image, allocates memory for it, and binds the given image and image memory

	VkResult result = vkCreateImage(m_device, &in_info, nullptr, &in_image);
	assert(result == VK_SUCCESS && "vkCreateImage() did not return success");

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_device, in_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, in_properties);

	result = vkAllocateMemory(m_device, &allocInfo, nullptr, &in_imageMemory);
	assert(result == VK_SUCCESS && "vkAllocateMemory() did not return success");

	vkBindImageMemory(m_device, in_image, in_imageMemory, 0);
}
void Vulkan::ChangeImageLayout(VkImage& in_image, VkFormat in_format, VkImageLayout in_old, VkImageLayout in_new)
{
	// In order to use vkCmdCopyBufferToImage(), the image has to be in the correct layout

	VkCommandBuffer commandBuffer = BeginSingleTimeCommand(m_drawCommandPools[0]);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = in_old;
	barrier.newLayout = in_new;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = in_image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (in_old == VK_IMAGE_LAYOUT_UNDEFINED && in_new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (in_old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && in_new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommand(m_drawCommandPools[0], commandBuffer);
}
void Vulkan::CopyBufferToImage(VkBuffer& in_buffer, VkImage& in_image, uint32_t in_width, uint32_t in_height)
{
	VkCommandBuffer command = BeginSingleTimeCommand(m_drawCommandPools[0]);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { in_width, in_height, 1 };

	vkCmdCopyBufferToImage(
		command,
		in_buffer,
		in_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Assuming the image layout has already been transitioned to this
		1,
		&region
	);

	EndSingleTimeCommand(m_drawCommandPools[0], command);
}
void Vulkan::CreateImageView(VkImageView& in_view, VkFormat in_format, VkImageAspectFlags in_aspectFlags, VkImage& in_image)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = in_image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = in_format;
	viewInfo.subresourceRange.aspectMask = in_aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &in_view);
	assert(result == VK_SUCCESS && "ERROR: vkCreateImageView() did not return success");
}


// ================================ Shader Functions ============================= //

std::vector<char> Vulkan::ReadFile(const std::string& in_fileName)
{
	// Reads all the bytes from the specified file and returns a vector of those bytes

	std::ifstream file(in_fileName, std::ios::ate | std::ios::binary); // Start reading at end and read in binary

	if (!file.is_open()) { throw std::runtime_error("ERROR: Failed to open file! (Check your shaders)"); }

	size_t fileSize = (size_t)file.tellg(); // Reading at the end to get size
	std::vector<char> out_buffer(fileSize);

	file.seekg(0); // Go back to the beginning and start reading
	file.read(out_buffer.data(), fileSize);

	file.close();

	return out_buffer;
}
VkShaderModule Vulkan::CreateShaderModule(const VkDevice in_device, const std::vector<char>& in_code)
{
	// Creates a shader module, basically a wrapper for the byte code, from the specified bytes

	std::cout << "Creating shader module..." << std::endl;

	VkShaderModule out_shaderModule{};

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = in_code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(in_code.data());

	VkResult result = vkCreateShaderModule(in_device, &createInfo, nullptr, &out_shaderModule);
	assert(result == VK_SUCCESS && "ERROR: vkCreateShaderModule() did not return success");

	return out_shaderModule;
}

// ================================ Helper Functions ============================= //

glm::vec3 Vulkan::NormalizeRGB(const glm::vec3& c)
{
	return glm::vec3(glm::abs(c.r) / 255.0f, glm::abs(c.g) / 255.0f, glm::abs(c.b) / 255.0f);
}

uint32_t Vulkan::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) // "Graphics cards can offer different types of memory to allocate from.Each type of
{ 																					   // memory varies in terms of allowed operationsand performance characteristics. We need to combine the
	VkPhysicalDeviceMemoryProperties memoryProps;									   // requirements of the bufferand our own application requirements to find the right type of memory to use."
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProps);

	for (uint32_t i = 0; i < memoryProps.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memoryProps.memoryTypes[i].propertyFlags & properties) == properties) { // typeFilter is a bit field, so checking if bit 'i' is set
			return i;
		}
	}

	assert(false && "ERROR: Could not find suitable memory type");

	return 0;
}

void Vulkan::CreateBuffer(VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkMemoryPropertyFlags in_props, VkBuffer& in_buffer, VkDeviceMemory& in_bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = in_size;
	bufferInfo.usage = in_usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &in_buffer);
	assert(result == VK_SUCCESS && "ERROR: vkCreateBuffer() did not return success"); // Check this

	// Buffer is created, but doesn't actually have any memory assigned to it yet
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, in_buffer, &memRequirements);

	// Allocate the memory
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, in_props);

	result = vkAllocateMemory(m_device, &allocInfo, nullptr, &in_bufferMemory);
	assert(result == VK_SUCCESS && "ERROR: vkAllocateMemory() in Vulkan::CreateVertexBuffer() did not return success");
	result = vkBindBufferMemory(m_device, in_buffer, in_bufferMemory, 0);
	assert(result == VK_SUCCESS && "ERROR: vkBindBufferMemory() did not return success");
}
void Vulkan::CopyBuffer(VkBuffer in_srcBuffer, VkBuffer in_dstBuffer, VkDeviceSize in_size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommand(m_drawCommandPools[0]);

	VkBufferCopy copyRegion{};
	copyRegion.size = in_size;
	copyRegion.dstOffset = 0;
	vkCmdCopyBuffer(commandBuffer, in_srcBuffer, in_dstBuffer, 1, &copyRegion);

	EndSingleTimeCommand(m_drawCommandPools[0], commandBuffer);
}