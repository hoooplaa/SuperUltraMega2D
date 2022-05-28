#pragma once

#include <vector>
#include <memory>

#include "Engine/Graphics/Objects/Batch.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Core/SystemGuard.h"
#include "Engine/AnimationPlayer.h"
#include "Engine/Map.h"

namespace Mega
{
	class Renderer;
	struct Batch;
};
struct GLFWwindow;
class Camera;

namespace Mega
{
	class Scene : public SystemGuard {
	public:
		friend Renderer;

		using tBatches = std::vector<Batch>;

		void Initialize() override;
		void Destroy() override;

		void Update(const float in_dt);

		void Clear();
		void Display(const std::shared_ptr<Camera> in_camera);
		void Display();

		void Draw(const Sprite& in_sprite);
		void Draw(const Map& in_map);
		void Draw(const AnimationPlayer& in_animPlayer);

		Map LoadMap(const std::string& in_dir);
		AnimationPlayer LoadAnimationPlayer(const std::string& in_dir);

		GLFWwindow* GetWindow() { return m_pWindow; }
		void SetRenderer(Renderer* in_pRenderer) { m_pRenderer = in_pRenderer; }

	private:
		inline Batch& GetBatchForTexture(const Texture& in_t);

		// Graphics
		GLFWwindow* m_pWindow = nullptr;
		Renderer* m_pRenderer = nullptr;

		tBatches m_batches;
	};
}