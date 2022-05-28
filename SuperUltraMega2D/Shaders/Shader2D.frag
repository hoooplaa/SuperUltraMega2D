#version 450

// --- IN --- //
layout(location = 0) in vec4 inFragColor;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec2 inTexIndexAndType;
layout(location = 3) in vec3 inFragPos;
layout(location = 4) in vec3 inNormal;

// --- OUT--- //
layout(location = 0) out vec4 outFragColor;

// --- Objects --- //
struct Light
{
	float constant;
	float linear;
	float quadratic;
	float strength;

	float inCutOff;
	float outCutOff;
	float distInCutOff;
	float distOutCutOff;

	vec3 position;
	vec3 direction;
	vec3 color;
};

// --- IN --- //
layout(std140, binding = 1) uniform UniformBufferObjectFrag {
	vec3 viewPos;
    vec3 viewDir;

	int lightCount;
	Light lights[10];
} ubo;

layout(binding = 2) uniform sampler2D texSampler[10];

// --- MAIN --- //
void main() {
	vec3 combinedLight = vec3(0.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < ubo.lightCount; ++i)
	{
		float attenuation = 0.0f;

		vec3 lightPos = ubo.lights[i].position;
		vec3 lightDir = ubo.lights[i].direction;
		float distance = length(lightPos - inFragPos);

		if (distance <= ubo.lights[i].distOutCutOff)
		{
			float c = ubo.lights[i].constant;
			float l = ubo.lights[i].linear;
			float q = ubo.lights[i].quadratic;

			vec2 toLight = normalize(vec2(lightPos) - vec2(inFragPos));
			float theta = dot(toLight, normalize(vec2(-lightDir)));

			// Is blocked //
			bool isBlocked = false;

			//for (int i = 0; i <= 30; i += 2)
			//{
			//	vec2 p1 = walls[i];
			//	vec2 p2 = walls[i + 1];
			//	vec2 edgeDir = p2 - p1;
			//	vec2 rayDir = vec2(inFragPos) - vec2(lightPos);

			//	float scale2 = (rayDir.x * (p1.y - lightPos.y) + rayDir.y * (lightPos.x - p1.x)) / (edgeDir.x * rayDir.y - edgeDir.y * rayDir.x);
			//	float scale1 = (p1.x + edgeDir.x * scale2 - lightPos.x) / rayDir.x;

			//	if (((scale1 >= 0 && scale1 <= 1.0) && (scale2 >= 0 && scale2 <= 1.0)) )
			//	{
			//		isBlocked = true;
			//		break;
			//	}
			// }
			// ---------- //

			if (!isBlocked)
			{
				if (theta > ubo.lights[i].outCutOff)
				{
					float radiusFade = ubo.lights[i].inCutOff - ubo.lights[i].outCutOff;
					float distFade   = ubo.lights[i].distInCutOff - ubo.lights[i].distOutCutOff;
					float radiusIntensity = clamp((theta - ubo.lights[i].outCutOff) / radiusFade, 0.0, 1.0); 
					float distIntensity   = clamp((distance - ubo.lights[i].distOutCutOff) / distFade, 0.0, 1.0);

					attenuation = 1.0 / (c + l * distance + q * (distance * distance)); 

					attenuation *= radiusIntensity * distIntensity;
				}
			}
		}

		combinedLight += attenuation * ubo.lights[i].strength;
	}

    // Finalize
    int index = int(inTexIndexAndType.x);

    if (index < 0) {
        outFragColor = inFragColor;
    }
    else {
    	outFragColor = texture(texSampler[index], inFragTexCoord) * inFragColor;
    }

   // outFragColor.rgb *= combinedLight;
}