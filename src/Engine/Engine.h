#pragma once

#include "Engine/Core/SystemGuard.h"
#include "Engine/Core/Debug.h"

namespace Mega
{
	class Renderer;
	class Scene;
}
struct GLFWwindow;

namespace Mega
{
	class Engine : public SystemGuard
	{
	public:
		void Initialize() override;
		void Destroy() override;

		inline Renderer* GetRenderer() { MEGA_ASSERT(IsInitialized(), "Engine not initialized"); return m_pRenderer; }
		inline Scene* GetScene() { MEGA_ASSERT(IsInitialized(), "Engine not initialized"); return m_pScene; }
		inline GLFWwindow* GetApplicationWindow() { MEGA_ASSERT(IsInitialized(), "Engine not initialized"); return m_pAppWindow; }
	private:
		Renderer* m_pRenderer;
		Scene* m_pScene;
		GLFWwindow* m_pAppWindow;
	};
}