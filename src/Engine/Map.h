#pragma once

#include <string>
#include <vector>

#include <GLM/vec2.hpp>
#include <GLM/vec3.hpp>

#include "Graphics/Objects/Texture.h"
#include "Graphics/Objects/Batch.h"
#include "Core/SystemGuard.h"

namespace Mega { class Renderer; }

namespace Mega
{
	// A class used for storing and loading map data from Tiled
	// Note: map vertex data is stored in the constant vertex buffer
	// and therefore map vertex data can not be edited as sprites' can
	class Map
	{
	public:
		friend Renderer;

		// Stores the texture coords for a specific tile used in the map
		struct Tile
		{
			// 00 = bottom left, 11 = top right
			Vec2F coord00;
			Vec2F coord10;
			Vec2F coord01;
			Vec2F coord11;
		};

		bool LoadData(const std::string& in_dir);

		void SetTileMap(const Texture& in_t) { m_tileMap = in_t; }
		void SetScale(float in_scale) { m_scale = in_scale; }
		const Batch& GetDrawBatch() const { return m_drawBatch; }
		const Texture& GetTileMap() const { return m_tileMap; }

	private:
		bool LoadMapData(const std::string& in_filepath);
		bool LoadTileSetData(const std::string& in_filepath);
		bool CreateBatch();

		Texture m_tileMap;
		float m_scale = 0.1f;

		std::vector<std::vector<int>> m_tileData;
		std::vector<Tile> m_tileTypes;
		Batch m_drawBatch;

		uint16_t m_tileCountX = 0;
		uint16_t m_tileCountY = 0;
		uint16_t m_tileWidth = 0;
		uint16_t m_tileHeight = 0;
		TEXTURE_DIMENSION_T m_spriteSheetWidth = 0;
		TEXTURE_DIMENSION_T m_spriteSheetHeight = 0;
	};
}