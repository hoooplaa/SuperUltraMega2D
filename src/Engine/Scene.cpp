#include "Scene.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "Engine/Graphics/Renderer.h"

namespace Mega
{
	void Scene::Initialize()
	{
		SystemGuard::Initialize();

		////////////////// Check this maybe textureless draws should be index 0? what index is it? also maybe batches on the heap
		Batch batch;
		m_batches = tBatches(MAX_TEXTURE_COUNT + 1, batch); // One batch for each texture + a batch for textureless draws
	}

	void Scene::Destroy()
	{
		SystemGuard::Destroy();
	}

	void Scene::Update(const float in_dt)
	{

	}

	void Scene::Clear()
	{
		for (auto& batch : m_batches)
		{
			batch.vertices.clear();
		}
	}

	void Scene::Display(const std::shared_ptr<Camera> in_camera)
	{
		
	}

	void Scene::Display()
	{
		m_pRenderer->DisplayScene(this);
	}

	void Scene::Draw(const Sprite& in_sprite)
	{
		// Get proper batch for the texture we're using
		Batch& batch = GetBatchForTexture(in_sprite.texture);

		Vec2F d = in_sprite.dimensions;
		Vec2F p = in_sprite.position;
		Vec2F td = in_sprite.swatch;
		Vec2F ts = in_sprite.coords;

		// Bottom left triangle
		Vertex v1; // Top left
		v1.texCoord = { ts.x, ts.y };
		v1.pos = { p.x, p.y, Z_COORD };
		Vertex v2; // Bottom left
		v2.texCoord = { ts.x, ts.y + td.y };
		v2.pos = { p.x, p.y - d.y, Z_COORD };
		Vertex v3; // Bottom right
		v3.texCoord = { ts.x + td.x, ts.y + td.y };
		v3.pos = { p.x + d.x, p.y - d.y, Z_COORD };

		// Top right triangle
		Vertex v4 = v1; // Top left
		Vertex v5; // Top right
		v5.texCoord = { ts.x + td.x, ts.y };
		v5.pos = { p.x + d.x, p.y, Z_COORD };
		Vertex v6 = v3; // Bottom right

		batch.vertices.push_back(v1);
		batch.vertices.push_back(v2);
		batch.vertices.push_back(v3);
		batch.vertices.push_back(v4);
		batch.vertices.push_back(v5);
		batch.vertices.push_back(v6);
		batch.texture = in_sprite.texture;
	}

	void Scene::Draw(const Map& in_map)
	{
		m_batches[0] = in_map.GetDrawBatch();
	}

	void Scene::Draw(const AnimationPlayer& in_animPlayer)
	{
		Draw(in_animPlayer.GetSprite());
	}

	Map Scene::LoadMap(const std::string& in_dir)
	{
		return m_pRenderer->LoadMap(in_dir);
	}

	AnimationPlayer Scene::LoadAnimationPlayer(const std::string& in_dir)
	{
		// Load in SpriteSheet.png
		Texture spriteSheet = m_pRenderer->LoadTexture(std::string(in_dir + "/SpriteSheet.png").c_str());

		AnimationPlayer out_animPlayer;
		out_animPlayer.SetSpriteSheet(spriteSheet);
		out_animPlayer.LoadAnimations(in_dir);

		return out_animPlayer;
	}

	Batch& Scene::GetBatchForTexture(const Texture& in_t)
	{
		// Should already be a batch for each possible texture so we simply get the proper batch for the texture
		MEGA_ASSERT(in_t.index < m_batches.size(), "Too many textures loaded or GetBatchForTexture() function fucking up tell alex");

		return m_batches[in_t.index];
	}
}