#pragma once

void math3d_vec3_set(float v[3], float x, float y, float z);
void math3d_vec3_copy(float out[3], const float in[3]);
void math3d_vec3_normalize(float v[3]);
void math3d_mat4_identity(float matrix[4][4]);
void math3d_mat4_multiply(float matrix[4][4], const float a[4][4], const float b[4][4]);
void math3d_mat4_model(float matrix[4][4], const float position[3], const float rotation[3],
					   const float scale[3]);
