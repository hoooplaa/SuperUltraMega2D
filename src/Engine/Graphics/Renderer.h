#pragma once

#include <string>

#include "Engine/Map.h"
#include "Engine/Core/SystemGuard.h"
#include "Engine/Graphics/Objects/Texture.h"

namespace Mega { class Scene; };
class Vulkan;
struct GLFWwindow;

namespace Mega
{
	class Renderer : public SystemGuard {
	public:
		Renderer();
		~Renderer();

		void Initialize() override;
		void Destroy() override;

		void DisplayScene(Scene* in_pScene) const;

		Texture LoadTexture(const std::string& in_filepath);
		Map LoadMap(const std::string& in_dir);

		void SetWindow(GLFWwindow* in_pWindow) { m_pWindow = in_pWindow; }
	private:
		Vulkan* m_pVulkanInstance;
		GLFWwindow* m_pWindow;

		uint8_t m_bitFieldRenderFlags = 0;
	};
}