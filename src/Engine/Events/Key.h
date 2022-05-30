#pragma once

#include <GLFW/glfw3.h>

namespace Mega
{
	enum class eKeyMode
	{
		Press = GLFW_PRESS,
		Release = GLFW_RELEASE,
		Repeat = GLFW_REPEAT,
	};

	struct Key
	{
		void GetScanCode()
		{
			glfwGetKeyScancode(1);
		}
	};
}