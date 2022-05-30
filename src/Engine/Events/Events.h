#pragma once

#include <GLFW/glfw3.h>

#include "Engine/Events/Key.h"

/*void a(GLFWwindow* a, int b, int c, int d, int e) {};
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		activate_airship();
}*/

namespace Mega
{
	class Engine;

	class Input
	{
	public:
		friend Engine;

		static void PollEvents()
		{
			glfwPollEvents();
		}

		//static bool IsKeyPressed(int keyCode)
		//{
			//return glfwGetKey(s_window, keyCode) == GLFW_PRESS;
		//}
	
	private:
		static GLFWwindow* s_window;

	};
}