#version 450

layout(std140, binding = 0) uniform UniformBufferObjectVert {
    mat4 view;
    mat4 proj;
} ubo;

layout( push_constant ) uniform constants {
    mat4 model;
    vec4 objectColor;

    vec2 texCoordAdd;
    vec2 texCoordMult;

    int textureIndex;
} push;

// IN
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

// OUT
layout(location = 0) out vec4 outFragColor;
layout(location = 1) out vec2 outFragTexCoord;
layout(location = 2) out vec2 outTexIndexAndType;
layout(location = 3) out vec3 outFragPos;
layout(location = 4) out vec3 outNormal;

layout(location = 5) out vec2 outTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition.x, inPosition.y, 0, 1.0);
    
    // Out
    outFragTexCoord = inTexCoord;
    outFragTexCoord *= push.texCoordMult;
    outFragTexCoord += push.texCoordAdd;

    outTexIndexAndType.x = push.textureIndex;
    outFragColor = inColor * push.objectColor;

    outFragPos = vec3(push.model * vec4(inPosition, 1.0)); // How do this work?
    outNormal  = mat3(push.model) * inNormal;
}