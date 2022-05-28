#include "Renderer.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>

#include "Engine/Scene.h"
#include "Engine/Core/SystemGuard.h"
#include "Engine/Core/Debug.h"
#include "Engine/Core/Math/Math.h"
#include "Engine/Graphics/Vulkan/Vulkan.h"

namespace Mega
{
	Renderer::Renderer()
	{

	}

	Renderer::~Renderer()
	{
		MEGA_ASSERT(IsDestroyed(), "Deleting Renderer before it has been properly destroyed");
	}

	void Renderer::Initialize()
	{
		SystemGuard::Initialize();

		std::cout << "Initializing Renderer..." << std::endl;

		MEGA_ASSERT(m_pWindow, "Inititalizing Mega::Renderer without a window");

		// Initialize Vulkan Graphics
		m_pVulkanInstance = new Vulkan;
		m_pVulkanInstance->Initialize(this, m_pWindow);
	}

	void Renderer::Destroy()
	{
		// Make sure we have been initilaized and not destroying early/accidentally
		MEGA_ASSERT(IsInitialized(), "Destroying Renderer before it has been initialized");

		// Cleanup our Vulkan instance
		m_pVulkanInstance->Destroy();
		delete m_pVulkanInstance;

		SystemGuard::Destroy();
	}

	void Renderer::DisplayScene(Scene* in_pScene) const
	{
		MEGA_ASSERT(IsInitialized(), "Displaying scene with unitialized renderer");
		MEGA_ASSERT(in_pScene->IsInitialized(), "Displaying scene that is not initialized");

		const Camera::ViewData& viewData = Camera::GetConstViewData();
		m_pVulkanInstance->SetViewData(viewData);

		m_pVulkanInstance->DrawFrame(in_pScene->m_batches);
	}

	Texture Renderer::LoadTexture(const std::string& in_filepath)
	{
		MEGA_ASSERT(IsInitialized(), "Trying to load texture while renderer is not initialized");
		Texture out_texture;

		m_pVulkanInstance->LoadTextureData(in_filepath.c_str(), &out_texture);
		m_pVulkanInstance->UpdateLoadedTextureData();

		return out_texture;
	}

	Map Renderer::LoadMap(const std::string& in_dir)
	{
		MEGA_ASSERT(IsInitialized(), "Trying to load map while renderer is not initialized");
		Map out_map;

		// Load map data
		// std::string textureSource = in_dir + "/" + image.attribute("source").as_string();
		Texture tileMap = LoadTexture(std::string(in_dir + "/map.png").c_str());
		out_map.SetTileMap(tileMap);
		out_map.LoadData(in_dir);

		// Add loaded batch to constant vertex data

		return out_map;
	}
}