#pragma once

#include <cstdint>

#define TEXTURE_INDEX_T int8_t
#define TEXTURE_DIMENSION_T uint16_t
#define MAX_TEXTURE_COUNT 10

namespace Mega
{
	struct Texture
	{
		TEXTURE_INDEX_T index = -1; // Index at which it is stored in texture array
									// (negative number meaning empty texture)
		TEXTURE_DIMENSION_T width = 0;
		TEXTURE_DIMENSION_T height = 0;

		inline bool static IsLoaded(const Texture& in_t) { return in_t.index >= 0; }
	};
}