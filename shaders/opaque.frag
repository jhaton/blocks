#version 450

layout(location = 0) in flat uint i_voxel;
layout(location = 1) in vec4 i_position;
layout(location = 2) in vec3 i_uv;
layout(location = 0) out vec4 o_position;
layout(location = 1) out vec2 o_uv;
layout(location = 2) out uint o_voxel;
layout(set = 2, binding = 0) uniform sampler2DArray s_atlas;

void main()
{
    if (texture(s_atlas, i_uv).a < 0.001)
    {
        discard;
    }
    o_position = i_position;
    o_uv = i_uv.xy;
    o_voxel = i_voxel;
}
