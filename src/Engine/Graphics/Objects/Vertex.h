#pragma once

#include <array>
#include <cstdint>

#include "Engine/Core/Math/Vec.h"

#define Z_COORD 0.0
#define VERTEX_INDEX_T uint32_t

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

namespace Mega
{
	struct Vertex {
		Vec3F pos = { 0.0f, 0.0f, 0.0f };
		Vec4F color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Vec2F texCoord = { 0.0f, 0.0f };

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};
}