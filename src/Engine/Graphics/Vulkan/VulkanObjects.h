#pragma once

#include <array>
#include <optional>

#include "VulkanInclude.h"
#include "VulkanDefines.h"
#include "Engine/Graphics/Objects/Light.h"
#include "Engine/Graphics/Objects/Vertex.h"

struct ImguiVertex {
	glm::vec4 pos;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription out_bindingDescription{};
		out_bindingDescription.binding = 0;
		out_bindingDescription.stride = sizeof(ImguiVertex);
		out_bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return out_bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 1> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 1> out_attributeDescriptions{};

		out_attributeDescriptions[0].binding = 0;
		out_attributeDescriptions[0].location = 0;
		out_attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		out_attributeDescriptions[0].offset = offsetof(ImguiVertex, pos);

		return out_attributeDescriptions;
	}
};

struct UniformBufferObjectVert {
	glm::mat4 view;
	glm::mat4 proj;
};
struct UniformBufferObjectFrag {
	alignas(sizeof(Mega::Light::glScalarF) * 4) glm::vec3 viewPos;
	alignas(sizeof(Mega::Light::glScalarF) * 4) glm::vec3 viewDir;

	uint32_t lightCount = 0;
	Mega::Light lights[MAX_LIGHT_COUNT];
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
struct QueueFamilyIndices {
	bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }

	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;
};

struct CommandObject {
	static void Destroy(VkDevice* in_pDevice, CommandObject* in_pCommand) {

	}

	VkCommandPool pool;
	VkCommandBuffer buffer;
};
struct ImageObject {
	static void Destroy(VkDevice* in_pDevice, ImageObject* in_pImage)
	{
		vkDestroyImage(*in_pDevice, in_pImage->image, nullptr);
		vkDestroyImageView(*in_pDevice, in_pImage->view, nullptr);
		vkFreeMemory(*in_pDevice, in_pImage->memory, nullptr);
	}

	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
	glm::vec2 extent;
};