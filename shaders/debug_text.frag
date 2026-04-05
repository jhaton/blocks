#version 450

layout(location = 0) in vec2 i_uv;
layout(location = 1) in vec4 i_color;
layout(location = 0) out vec4 o_color;

layout(set = 2, binding = 0) uniform sampler2D s_font;

void main()
{
    float alpha = texture(s_font, i_uv).r;
    if (alpha <= 0.0)
    {
        discard;
    }
    o_color = vec4(i_color.rgb, i_color.a * alpha);
}
