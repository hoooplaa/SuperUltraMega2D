#include "Map.h"

#include <iostream>
#include <pugixml.hpp>

#define ERROR_MSG(msg) std::cout << msg << std::endl;
#define MAP_START_POS 0, 0

namespace Mega
{
	// Loads map data from given map data directory, returns true on success false otherwise
	// Note: this simply loads the raw map data which does not include the texture used for the tile map
	// which if not loading through the renderer must still be set (aka load the map through the renderer)
	bool Map::LoadData(const std::string& in_dir)
	{
		std::cout << "Loading Map Data from " << in_dir << "..." << std::endl;

		bool result1 = LoadMapData(in_dir);
		MEGA_ASSERT(result1, "LoadMapData() did not return true");

		bool result2 = LoadTileSetData(in_dir);
		MEGA_ASSERT(result2, "LoadTileSetData() did not return true");

		bool result3 = CreateBatch();
		MEGA_ASSERT(result3, "CreateBatch() did not return true");

		std::cout << "Map Loaded" << std::endl;
		return result1 && result2 && result3;
	}

	//////////////////////// Still need to add tile scale (rn defaulted at 1) in LoadTileSetData()
	//////////////////////// Loading tile types wrong just in a row? idk

	///////////////////////////////// Private ///////////////////////////////////////

	// Takes in directory with tiled .tmx file and loads proper map data
	bool Map::LoadMapData(const std::string& in_dir)
	{
		// Load doc
		pugi::xml_document tmx;
		pugi::xml_parse_result result = tmx.load_file(std::string(in_dir + "/map.tmx").c_str());
		if (!result) {
			ERROR_MSG(result.description());
			return false;
		}

		// Parse map metadata and store
		auto map = tmx.child("map");
		m_tileCountX = map.attribute("width").as_int();
		m_tileCountY = map.attribute("height").as_int();

		// Parse map tile data and store
		auto layer = map.child("layer");
		auto data = layer.child("data");

		const std::string str = data.text().as_string();

		int tc = 0; // num of tiles we have gone through (only incremented if we get past the c == '\n' shit)
		m_tileData.resize(m_tileCountY);
		for (int i = 0; i < str.length(); ++i)
		{
			char c = str[i];
			if (c == '\n') continue;
			if (c == ',')  continue;

			int row = tc / m_tileCountX;
			int col = tc % m_tileCountX;
			tc++;

			m_tileData[row].push_back(c - '0'); // sexy char magic, ascii nums start at 48 (same as c - 48)
		}
		// Print shit
		for (int row = 0; row < m_tileData.size(); row++)
		{
			for (int col = 0; col < m_tileData[row].size(); col++)
			{
				std::cout << m_tileData[row][col] << ", ";
			}
			std::cout << "\n";
		}
		std::cout << std::flush;

		return true;
	}

	// Takes in directory with tiled .tsx and loads data for the tileset
	bool Map::LoadTileSetData(const std::string& in_dir)
	{
		pugi::xml_document tsx;
		pugi::xml_parse_result result = tsx.load_file(std::string(in_dir + "/map.tsx").c_str());
		if (!result) {
			ERROR_MSG(result.description());
			return false;
		}

		auto tileset = tsx.child("tileset");
		auto image = tileset.child("image");
		m_tileWidth = tileset.attribute("tilewidth").as_int();
		m_tileHeight = tileset.attribute("tileheight").as_int();

		int tileCount = tileset.attribute("tilecount").as_int();
		m_spriteSheetWidth = image.attribute("width").as_int();
		m_spriteSheetHeight = image.attribute("height").as_int();

		// Create each tile and set proper texture coords
		MEGA_ASSERT(m_spriteSheetWidth % m_tileWidth == 0, "Map spritesheet size and tile size do not match");
		MEGA_ASSERT(m_spriteSheetHeight % m_tileHeight == 0, "Map spritesheet size and tile size do not match");
		{
			float row = 0; // row is float to make sure texCoord arithmetic returns floats
			for (row = 0; row * m_tileWidth < m_spriteSheetWidth; row++)
			{
				// Get texture coords for tile type

				Tile newTile;
				newTile.coord00 = { row * m_tileWidth / m_spriteSheetWidth, 0 }; // Top left
				newTile.coord10 = { (row + 1) * m_tileWidth / m_spriteSheetWidth, 0 }; // Top right
				newTile.coord01 = { row * m_tileWidth / m_spriteSheetWidth, m_tileHeight / m_spriteSheetHeight }; // Bottom left
				newTile.coord11 = { (row + 1) * m_tileWidth / m_spriteSheetWidth, m_tileHeight / m_spriteSheetHeight }; // Bottom right

				// std::cout << "SW: " << m_spriteSheetWidth << std::endl;
				// std::cout << "Width: " << m_tileWidth << std::endl;
				// std::cout << "Height: " << m_tileHeight << std::endl;
				// std::cout << "row: " << row << std::endl;
				// std::cout << "00 " << newTile.coord00.x << ", " << newTile.coord00.y << std::endl;
				// std::cout << "01 " << newTile.coord01.x << ", " << newTile.coord01.y << std::endl;
				// std::cout << "10 " << (row + 1) * m_tileWidth / m_spriteSheetWidth << ", " << newTile.coord10.y << std::endl;
				// std::cout << "11 " << newTile.coord11.x << ", " << newTile.coord11.y << std::endl;

				m_tileTypes.push_back(newTile);
			}
			MEGA_ASSERT(row == tileCount, "Error loading tile types (loaded tile count did not match expected tile count)");
		}

		return true;
	}

	// Take the loaded map and tileset data and turn it into a draw batch to be rendered by the engine
	bool Map::CreateBatch()
	{
		// Add texture data
		m_drawBatch.texture = m_tileMap;
		
		// Create vertex data
		for (float col = 0; col < m_tileData.size(); col++)
		{
			for (float row = 0; row < m_tileData[col].size(); row++) // floats to ensure proper vertex arithmetic
			{
				Tile tileType = m_tileTypes[m_tileData[row][col] - 1]; // Tile types start at index 1

				// Make quad
				Vertex v1;
				Vertex v2;
				Vertex v3;
				Vertex v4;
				Vertex v5;
				Vertex v6;

				// Texture coords
				v1.texCoord = tileType.coord00;
				v2.texCoord = tileType.coord01;
				v3.texCoord = tileType.coord11;
				v4.texCoord = tileType.coord00;
				v5.texCoord = tileType.coord10;
				v6.texCoord = tileType.coord11;

				// Position
				v1.pos = { row * m_scale, col * m_scale, Z_COORD };
				v2.pos = { row * m_scale, (col - 1) * m_scale, Z_COORD };
				v3.pos = { (row + 1) * m_scale, (col - 1) * m_scale, Z_COORD };
				v4.pos = { row * m_scale, col * m_scale, Z_COORD };
				v5.pos = { (row + 1) * m_scale, col * m_scale, Z_COORD };
				v6.pos = { (row + 1) * m_scale, (col - 1) * m_scale, Z_COORD };

				m_drawBatch.vertices.push_back(v1);
				m_drawBatch.vertices.push_back(v2);
				m_drawBatch.vertices.push_back(v3);
				m_drawBatch.vertices.push_back(v4);
				m_drawBatch.vertices.push_back(v5);
				m_drawBatch.vertices.push_back(v6);
			}
		}

		return true;
	}
}