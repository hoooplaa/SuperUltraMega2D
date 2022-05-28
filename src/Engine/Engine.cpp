#include "Engine.h"

#include <GLFW/glfw3.h>

#include "Engine/Graphics/Renderer.h"
#include "Engine/Scene.h"

namespace Mega
{
	void Engine::Initialize()
	{
		SystemGuard::Initialize();

		// Initialize GLFW and create our application window window
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Telling it not to create an OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // For now telling GLFW we dont want the window to be resized

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		m_pAppWindow = glfwCreateWindow(mode->width, mode->height, "SuperUltraMega2D", nullptr, nullptr); // 4th param lets you optionally specify a monitor to display

		// Initialize our systems
		m_pRenderer = new Renderer;
		m_pRenderer->SetWindow(m_pAppWindow);
		m_pRenderer->Initialize();

		m_pScene = new Scene;
		m_pScene->SetRenderer(m_pRenderer);
		m_pScene->Initialize();
	}

	void Engine::Destroy()
	{
		// Clenup our systems
		m_pScene->Destroy();
		m_pRenderer->Destroy();

		delete m_pScene;
		delete m_pRenderer;

		// Delete our application window
		glfwDestroyWindow(m_pAppWindow);
		glfwTerminate();
		
		SystemGuard::Destroy();
	}
}