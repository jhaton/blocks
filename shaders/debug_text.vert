#version 450

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;

layout(location = 0) out vec2 o_uv;
layout(location = 1) out vec4 o_color;

layout(set = 1, binding = 0) uniform debug_text_uniforms
{
    vec4 u_viewport;
};

void main()
{
    vec2 clip;
    clip.x = (i_position.x / u_viewport.z) * 2.0 - 1.0;
    clip.y = 1.0 - (i_position.y / u_viewport.w) * 2.0;
    gl_Position = vec4(clip, 0.0, 1.0);
    o_uv = i_uv;
    o_color = i_color;
}
