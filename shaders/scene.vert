#version 450

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;

layout(location = 0) out vec3 o_world_position;
layout(location = 1) out vec3 o_world_normal;
layout(location = 2) out vec4 o_shadow_position;

layout(set = 1, binding = 0) uniform scene_vertex_uniforms
{
    mat4 u_view_projection;
    mat4 u_model_matrix;
    mat4 u_light_matrix;
};

void main()
{
    vec4 world_position = u_model_matrix * vec4(i_position, 1.0);
    o_world_position = world_position.xyz;
    o_world_normal = normalize(mat3(u_model_matrix) * i_normal);
    o_shadow_position = u_light_matrix * world_position;
    gl_Position = u_view_projection * world_position;
}
