#include "Camera.h"

namespace Mega
{
	Vec3F MakeVec3(float x, float y, float z) {
		Vec3F out_v;
		out_v.x = x;
		out_v.y = y;
		out_v.z = z;
		return out_v;
	}
	Mat4x4F MakeMat4(float v) {
		Mat4x4F out_m;
		out_m[0][0] = v;
		out_m[0][1] = v;
		out_m[0][2] = v;
		out_m[0][3] = v;
		out_m[1][0] = v;
		out_m[1][1] = v;
		out_m[1][2] = v;
		out_m[1][3] = v;
		out_m[2][0] = v;
		out_m[2][1] = v;
		out_m[2][2] = v;
		out_m[2][3] = v;
		out_m[3][0] = v;
		out_m[3][1] = v;
		out_m[3][2] = v;
		out_m[3][3] = v;
		return out_m;
	}

	Mat4x4F MakeRotationMatrixX(float in_thetaRad) {
		Mat4x4F out_mat = MakeMat4(0.0f);
		out_mat[0][0] = 1.0f;
		out_mat[1][1] = cosf(in_thetaRad);
		out_mat[1][2] = sinf(in_thetaRad);
		out_mat[2][1] = -sinf(in_thetaRad);
		out_mat[2][2] = cosf(in_thetaRad);
		out_mat[3][3] = 1.0f;
		return out_mat;
	}
	Mat4x4F MakeRotationMatrixY(float in_thetaRad) {
		glm::mat4x4 out_mat = MakeMat4(0.0f);;
		out_mat[0][0] = cosf(in_thetaRad);
		out_mat[0][2] = sinf(in_thetaRad);
		out_mat[2][0] = -sinf(in_thetaRad);
		out_mat[1][1] = 1.0f;
		out_mat[2][2] = cosf(in_thetaRad);
		out_mat[3][3] = 1.0f;
		return out_mat;
	}
	Mat4x4F MakeRotationMatrixZ(float in_thetaRad) {
		glm::mat4x4 out_mat;
		out_mat[0][0] = cos(in_thetaRad);
		out_mat[0][1] = sinf(in_thetaRad);
		out_mat[1][0] = -sinf(in_thetaRad);
		out_mat[1][1] = cosf(in_thetaRad);
		out_mat[2][2] = 1.0f;
		out_mat[3][3] = 1.0f;
		return out_mat;
	}

	Vec3F MultiplyMatrixVector(const Mat4x4F& m, const Vec3F& v) {
		Vec3F out_vec;
		out_vec.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0]; // + v.w * m[3][0];
		out_vec.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1]; // + v.w * m[3][1];
		out_vec.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2]; // + v.w * m[3][2];
		//out_vec.w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + v.w * m[3][3];
		return out_vec;
	}

	// =========================== Camera Class =========================== //

	Camera::ViewData Camera::GetViewData() {
		ViewData out_view;

		out_view.eye = m_position;
		out_view.target = m_position + GetDirection();
		out_view.up = m_up;

		return out_view;
	}
	Vec3F Camera::GetDirection() {
		Vec3F out_dir = m_forward;
		out_dir = MultiplyMatrixVector(MakeRotationMatrixX(m_pitch), out_dir);
		out_dir = MultiplyMatrixVector(MakeRotationMatrixY(m_yaw), out_dir);

		return out_dir;
	}
	Vec3F Camera::GetUp() {
		Vec3F out_up = m_up;
		//out_up = MultiplyMatrixVector(MakeRotationMatrixZ(m_roll), out_up);

		return out_up;
	}

	void Camera::Forward(const float in_movement) {
		Vec3F vec = GetDirection();

		Move(vec * in_movement);
	}
	void Camera::Strafe(const float in_movement) {
		Vec3F dir = GetDirection();
		Vec3F vec = MakeVec3(dir.z, 0.0f, -dir.x);

		Move(vec * in_movement);
	}
	void Camera::Up(const float in_movement) {
		Vec3F up = GetUp();

		Move(up * in_movement);
	}

	Camera::ViewData Camera::GetConstViewData()
	{
		ViewData out_view;

		out_view.eye = Vec3F(CAMERA_POSITION);
		out_view.target = Vec3F(CAMERA_POSITION) + Vec3F(CAMERA_FORWARD);
		out_view.up = Vec3F(CAMERA_UP);

		return out_view;
	}
}