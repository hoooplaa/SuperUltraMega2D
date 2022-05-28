#pragma once

#include "Engine/Graphics/Renderer.h"
#include "Engine/Scene.h"
#include "Engine/Engine.h"

namespace Mega
{
	static Engine* CreateEngine()
	{
		Engine* out_pEngine = new Engine;
		out_pEngine->Initialize();

		return out_pEngine;
	}

	static void DestroyEngine(Engine* in_pEngine)
	{
		in_pEngine->Destroy();
		delete in_pEngine;
	}

	static Renderer* CreateRenderer()
	{
		Renderer* out_pRenderer = new Renderer;
		out_pRenderer->Initialize();

		return out_pRenderer;
	}

	static void DestroyRenderer(Renderer* in_pRenderer)
	{
		in_pRenderer->Destroy();
		delete in_pRenderer;
	}

	static Scene* CreateScene(Renderer* in_pRenderer)
	{
		Scene* out_pScene = new Scene;
		out_pScene->Initialize();

		return out_pScene;
	}

	static void DestroyScene(Scene* in_pScene)
	{
		in_pScene->Destroy();
		delete in_pScene;
	}
};