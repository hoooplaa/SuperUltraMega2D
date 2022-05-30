#pragma once

#include "Engine/Graphics/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Engine.h"
#include "Engine/Events/Events.h"

namespace Mega
{
	static Engine CreateEngine()
	{
		Engine out_engine;
		out_engine.Initialize();

		return out_engine;
	}

	static void DestroyEngine(Engine in_pEngine)
	{
		in_pEngine.Destroy();
	}
};