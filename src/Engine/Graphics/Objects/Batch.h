#pragma once

#include "Engine/Graphics/Objects/Texture.h"
#include "Engine/Graphics/Objects/Vertex.h"

#include <vector>

namespace Mega
{
	// Data structure used for storing vertex and texture data for a single batched draw call
	struct Batch
	{
		Batch();
		~Batch();

		Texture texture;
		std::vector<Vertex> vertices;
		VERTEX_INDEX_T vertexIndices[2] = { 0, 0 }; // start and end indexes for the batch in the vulkan vertex array
	};
}