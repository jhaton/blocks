#version 450

layout(location = 0) in vec3 i_position;

layout(set = 1, binding = 0) uniform shadow_vertex_uniforms
{
    mat4 u_light_matrix;
    mat4 u_model_matrix;
};

void main()
{
    gl_Position = u_light_matrix * u_model_matrix * vec4(i_position, 1.0);
}
