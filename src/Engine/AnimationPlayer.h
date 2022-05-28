#pragma once

#include <vector>
#include <string>
#include <cstdint>

#include "Engine/Core/Math/Vec.h"
#include "Engine/Core/SystemGuard.h"
#include "Engine/Graphics/Sprite.h"
#include "Engine/Graphics/Objects/Texture.h"

#define FRAME_INDEX_T uint32_t

namespace Mega
{
	class AnimationPlayer;
	class Scene;
}

namespace Mega
{
	struct Animation {
	public:
		struct Frame {
			FRAME_INDEX_T index = 0; // Index for frame in animation sequence

			Vec2F swatch = { 1.0f, 1.0f }; // Normalized (0 - 1) 2d dimensions of the area of the texture to draw from
			Vec2F coords = { 0.0f, 0.0f }; // Normalized (0 - 1) 2d starting point to draw that area ((0, 0) meaning top left of texture)
		};

		void AddFrame(const Frame& in_frame);
		uint32_t GetFrameCount() const { return frames.size(); }

		std::vector<Frame> frames;
		std::string name = "";
		float fps = 15.0f;
	};

	class AnimationPlayer {
	public:

		void Update(const float in_dt);

		void Pause();
		void Play();

		void  SetFlipped(bool in_flipped);
		Vec2F GetPosition() const { return m_sprite.position; }
		void  SetPosition(const Vec2F& in_pos) { m_sprite.position = in_pos; }

		void SetSpriteDimensions(const Vec2F& in_dims) { m_sprite.dimensions = in_dims; }
		void SetSpriteSheet(const Texture& in_spriteSheet) { m_spriteSheet = in_spriteSheet; m_sprite.texture = in_spriteSheet; }

		bool PlayAnimation(const std::string& in_name);
		bool LoadAnimations(const std::string& in_dirPath);

		const Animation* GetActiveAnimation() const { return m_pActiveAnimation; }
		
		const Sprite& GetSprite() const { return m_sprite; }
		FRAME_INDEX_T GetFrameIndex() const { return m_frameIndex; }

	private:
		void SetActiveAnimation(const Animation* in_psAnim);

		const Animation* m_pActiveAnimation = nullptr;
		std::vector<Animation> m_animations;

		FRAME_INDEX_T m_frameIndex;

		float m_tic = 0.0f;
		bool m_playing = true;
		bool m_flipped = false;

		Sprite m_sprite; // Sprite used for rendering animation data
		Texture m_spriteSheet;
	};
}