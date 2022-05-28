#pragma once

#include "Engine/Core/Math/Vec.h"
#include "Engine/Core/Math/Mat.h"

#define CAMERA_POSITION 0.0f, 0.0f, 5.0f
#define CAMERA_FORWARD  0.0f, 0.0f, -1.0f
#define CAMERA_UP       0.0f, 1.0f, 0.0f

namespace Mega
{
	class Camera {
	public:
		struct ViewData {
			Vec3F target;
			Vec3F eye;
			Vec3F up;
		};

		ViewData GetViewData();
		Vec3F GetDirection();
		Vec3F GetUp();
		Vec3F GetPosition() const { return m_position; }
		void SetPosition(const Vec3F& in_position) { m_position = in_position; }
		void SetPosition(const Vec2F& in_position) { m_position = Vec3F(in_position.x, in_position.y, GetPosition().z); }

		void Move(const Vec3F& in_movement) { m_position += in_movement; }
		void Strafe(const float in_movement);
		void Forward(const float in_movement);
		void Up(const float in_movement);

		void RotatePitch(const float in_rad) { m_pitch += in_rad; }
		void RotateYaw(const float in_rad) { m_yaw += in_rad; }
		//void RotateRoll(float in_rad)  { m_roll += in_rad; }

		static ViewData GetConstViewData();

	private:
		Vec3F m_position = Vec3F(CAMERA_POSITION);
		Vec3F m_forward = Vec3F(CAMERA_FORWARD);
		Vec3F m_up = Vec3F(CAMERA_UP);

		float m_pitch = 0.0f;
		float m_yaw = 0.0f;
		float m_roll = 0.0f;
	};
}