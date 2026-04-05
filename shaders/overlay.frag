#version 450

layout(location = 0) in vec2 i_uv;
layout(location = 0) out vec4 o_color;

layout(set = 3, binding = 0) uniform overlay_uniforms
{
    ivec4 u_viewport;
    ivec4 u_debug_state;
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

    float panel_height = max(52.0, 24.0 + float(u_debug_state.z) * 16.0) * scale;
    vec2 panel_min = vec2(18.0, 18.0) * scale;
    vec2 panel_max = panel_min + vec2(430.0, panel_height) * scale;
    bool in_panel = frag.x >= panel_min.x && frag.x <= panel_max.x && frag.y >= panel_min.y && frag.y <= panel_max.y;

    if (horizontal || vertical)
    {
        o_color = vec4(1.0, 1.0, 1.0, 0.92);
        return;
    }

    if (u_debug_state.x != 0 && u_debug_state.y != 0 && in_panel)
    {
        float local_x = frag.x - panel_min.x;
        float local_y = frag.y - panel_min.y;
        float border = 2.0 * scale;
        bool border_hit = local_x <= border || local_x >= (panel_max.x - panel_min.x) - border ||
            local_y <= border || local_y >= (panel_max.y - panel_min.y) - border;
        vec3 color = border_hit ? vec3(0.91, 0.73, 0.39) : vec3(0.11, 0.14, 0.18);
        float alpha = border_hit ? 0.92 : 0.78;
        if (alpha > 0.0)
        {
            o_color = vec4(color, alpha);
            return;
        }
    }

    discard;
}
