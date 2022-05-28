#pragma once

#define INDEX_TYPE uint32_t
#define SHADER_PATH_VERT "Shaders/vert2D.spv"
#define SHADER_PATH_FRAG "Shaders/frag2D.spv"

//#define CULL_MODE VK_CULL_MODE_BACK_BIT
#define CULL_MODE VK_CULL_MODE_NONE
#define DEPTH_BUFFERING false

#define CLEAR_COLOR 0/255.0f, 0/255.0f, 0/255.0f, 1.0f

#define MAX_LIGHT_COUNT 10

#define MAX_TEXTURE_COUNT 10
#define MAX_BONE_INFLUENCE 10