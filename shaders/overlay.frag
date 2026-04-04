#version 450

layout(location = 0) in vec2 i_uv;
layout(location = 0) out vec4 o_color;

layout(set = 3, binding = 0) uniform overlay_uniforms
{
    ivec4 u_viewport;
};

void main()
{
    vec2 frag = vec2(gl_FragCoord.x, float(u_viewport.w) - gl_FragCoord.y);
    vec2 center = vec2(u_viewport.z, u_viewport.w) * 0.5;
    float scale = min(float(u_viewport.z) / 1600.0, float(u_viewport.w) / 900.0);

    float cross_width = 10.0 * scale;
    float cross_thickness = 1.5 * scale;
    bool horizontal = abs(frag.x - center.x) <= cross_width && abs(frag.y - center.y) <= cross_thickness;
    bool vertical = abs(frag.y - center.y) <= cross_width && abs(frag.x - center.x) <= cross_thickness;

    vec2 panel_min = vec2(18.0, 18.0) * scale;
    vec2 panel_max = panel_min + vec2(132.0, 42.0) * scale;
    bool in_panel = frag.x >= panel_min.x && frag.x <= panel_max.x && frag.y >= panel_min.y && frag.y <= panel_max.y;

    if (horizontal || vertical)
    {
        o_color = vec4(1.0, 1.0, 1.0, 0.92);
        return;
    }

    if (in_panel)
    {
        float local_x = frag.x - panel_min.x;
        float local_y = frag.y - panel_min.y;
        vec3 color = vec3(0.14, 0.17, 0.21);
        if (local_y > 8.0 * scale && local_y < 14.0 * scale)
        {
            color = vec3(0.91, 0.73, 0.39);
        }
        else if (local_y > 18.0 * scale && local_y < 24.0 * scale)
        {
            color = vec3(0.45, 0.72, 0.98);
        }
        else if (local_y > 28.0 * scale && local_y < 34.0 * scale)
        {
            color = vec3(0.47, 0.82, 0.58);
        }
        float alpha = local_x < 118.0 * scale ? 0.65 : 0.0;
        if (alpha > 0.0)
        {
            o_color = vec4(color, alpha);
            return;
        }
    }

    discard;
}
