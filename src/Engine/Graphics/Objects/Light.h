#pragma once

#include "Engine/Core/Math/Math.h"

namespace Mega
{
	enum class eLightTypes {
		Directional,
		Point,
		Spotlight
	};

	struct Light
	{
		using glScalarF = float;
		using glScalarUI = uint32_t;

		glScalarF constant = 1.0f;
		glScalarF linear = 0.09f;
		glScalarF quadratic = 0.032f;
		glScalarF strength = 1.0f;

		glScalarF inCutOff = -PI / 2;
		glScalarF outCutOff = -PI / 2;
		glScalarF inCutOffDist = 10.0f;
		glScalarF outCutOffDist = 15.0f;

		alignas(sizeof(glScalarF) * 4) Vec3F  position = Vec3F(0.0f, 0.0f, 0.0f);
		alignas(sizeof(glScalarF) * 4) Vec3F  direction = Vec3F(1.0f, 0.0f, 1.0f);
		alignas(sizeof(glScalarF) * 4) Vec3UF color = Vec3UF(1.0f, 1.0f, 1.0f);
	};
}