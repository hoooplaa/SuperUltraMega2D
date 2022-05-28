#include "Vertex.h"

#include "Engine/Graphics/Vulkan/VulkanInclude.h"

VkVertexInputBindingDescription Mega::Vertex::GetBindingDescription()
{
	VkVertexInputBindingDescription out_bindingDescription{};
	out_bindingDescription.binding = 0;
	out_bindingDescription.stride = sizeof(Mega::Vertex);
	out_bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return out_bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> Mega::Vertex::GetAttributeDescriptions()
{
	std::array<VkVertexInputAttributeDescription, 4> out_attributeDescriptions{};

	out_attributeDescriptions[0].binding = 0;
	out_attributeDescriptions[0].location = 0;
	out_attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	out_attributeDescriptions[0].offset = offsetof(Mega::Vertex, pos);

	out_attributeDescriptions[1].binding = 0;
	out_attributeDescriptions[1].location = 1;
	out_attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	out_attributeDescriptions[1].offset = offsetof(Mega::Vertex, color);

	out_attributeDescriptions[2].binding = 0;
	out_attributeDescriptions[2].location = 2;
	out_attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	out_attributeDescriptions[2].offset = offsetof(Mega::Vertex, texCoord);

	out_attributeDescriptions[3].binding = 0;
	out_attributeDescriptions[3].location = 3;
	out_attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	out_attributeDescriptions[3].offset = offsetof(Mega::Vertex, normal);

	return out_attributeDescriptions;
}