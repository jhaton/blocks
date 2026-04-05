#version 450

layout(location = 0) in vec3 i_world_position;
layout(location = 1) in vec3 i_world_normal;
layout(location = 2) in vec4 i_shadow_position;

layout(location = 0) out vec4 o_color;

layout(set = 2, binding = 0) uniform sampler2D s_shadow;

layout(set = 3, binding = 0) uniform scene_fragment_uniforms
{
    vec4 u_light_direction;
    vec4 u_light_color_intensity;
    vec4 u_camera_position_time;
    vec4 u_albedo;
    vec4 u_interaction;
};

float sample_shadow(vec3 normal, vec3 light_dir)
{
    vec3 projected = i_shadow_position.xyz / max(i_shadow_position.w, 0.0001);
    projected = projected * 0.5 + 0.5;
    if (projected.x < 0.0 || projected.x > 1.0 ||
        projected.y < 0.0 || projected.y > 1.0 ||
        projected.z < 0.0 || projected.z > 1.0)
    {
        return 1.0;
    }

    float stored_depth = texture(s_shadow, projected.xy).r;
    float bias = max(0.0008, 0.0035 * (1.0 - max(dot(normal, light_dir), 0.0)));
    return projected.z - bias <= stored_depth ? 1.0 : 0.32;
}

void main()
{
    vec3 normal = normalize(i_world_normal);
    vec3 light_dir = normalize(-u_light_direction.xyz);
    vec3 view_vector = u_camera_position_time.xyz - i_world_position;
    float distance_to_camera = length(view_vector);

    float diffuse = max(dot(normal, light_dir), 0.0);
    float shadow = sample_shadow(normal, light_dir);
    float ambient = mix(0.22, 0.36, normal.y * 0.5 + 0.5);
    float rim = pow(1.0 - max(dot(normalize(view_vector), normal), 0.0), 2.5) * 0.08;

    vec3 color = u_albedo.rgb;
    if (u_interaction.y > 0.5)
    {
        color = mix(color, vec3(0.42, 0.92, 0.66), 0.22);
    }
    if (u_interaction.x > 0.5)
    {
        color = mix(color, vec3(0.98, 0.82, 0.38), 0.30);
    }
    vec3 light_color = u_light_color_intensity.rgb * u_light_color_intensity.a;
    vec3 lit = color * ambient;
    lit += color * diffuse * shadow * light_color;
    lit += rim * vec3(0.7, 0.8, 1.0);

    float fog = clamp(distance_to_camera / 42.0, 0.0, 1.0);
    vec3 fog_color = vec3(0.09, 0.12, 0.16);
    o_color = vec4(mix(lit, fog_color, fog * fog), 1.0);
}
