#pragma once

#include "Engine/Core/Math/Vec.h"
#include "Engine/Graphics/Objects/Texture.h"

namespace Mega
{
	// Rectanglular drawable object; dimensions start at top left corner
	struct Sprite
	{
	public:
		Sprite() {}
		Sprite(Texture in_texture)
			: texture(in_texture) {}
		Sprite(Texture in_texture, Vec2F in_swatch, Vec2F in_coords)
			: texture(in_texture), swatch(in_swatch), coords(in_coords) {}

		Texture texture; // Texture to draw from (defaults to none)

		Vec2F position   = Vec2F(0);
		Vec2F dimensions = Vec2F(1);
		Vec2F swatch     = Vec2F(1); // Normalized (0 - 1) 2d dimensions of the area of the texture
		Vec2F coords     = Vec2F(0); // Normalized (0 - 1) 2d starting point to draw texture from ((0, 0) meaning top left)
	};
}