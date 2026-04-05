#include "math3d.hpp"
#include "helpers.hpp"
#include <math.h>

void math3d_vec3_set(float v[3], float x, float y, float z) {
	assert(v);
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void math3d_vec3_copy(float out[3], const float in[3]) {
	assert(out);
	assert(in);
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void math3d_vec3_normalize(float v[3]) {
	assert(v);
	float length = 0.0f;
	length += v[0] * v[0];
	length += v[1] * v[1];
	length += v[2] * v[2];
	length = sqrtf(length);
	if (length <= EPSILON) {
		return;
	}
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

void math3d_mat4_identity(float matrix[4][4]) {
	assert(matrix);
	SDL_zeroa(*matrix);
	matrix[0][0] = 1.0f;
	matrix[1][1] = 1.0f;
	matrix[2][2] = 1.0f;
	matrix[3][3] = 1.0f;
}

void math3d_mat4_multiply(float matrix[4][4], const float a[4][4], const float b[4][4]) {
	assert(matrix);
	assert(a);
	assert(b);
	float c[4][4];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			c[i][j] = 0.0f;
			c[i][j] += a[0][j] * b[i][0];
			c[i][j] += a[1][j] * b[i][1];
			c[i][j] += a[2][j] * b[i][2];
			c[i][j] += a[3][j] * b[i][3];
		}
	}
	SDL_memcpy(matrix, c, sizeof(c));
}

void math3d_mat4_model(float matrix[4][4], const float position[3], const float rotation[3],
					   const float scale[3]) {
	assert(matrix);
	assert(position);
	assert(rotation);
	assert(scale);

	const float sx = sinf(rotation[0]);
	const float cx = cosf(rotation[0]);
	const float sy = sinf(rotation[1]);
	const float cy = cosf(rotation[1]);
	const float sz = sinf(rotation[2]);
	const float cz = cosf(rotation[2]);

	matrix[0][0] = (cy * cz + sy * sx * sz) * scale[0];
	matrix[0][1] = (cx * sz) * scale[0];
	matrix[0][2] = (cy * sx * sz - cz * sy) * scale[0];
	matrix[0][3] = 0.0f;

	matrix[1][0] = (cz * sy * sx - cy * sz) * scale[1];
	matrix[1][1] = (cx * cz) * scale[1];
	matrix[1][2] = (cy * cz * sx + sy * sz) * scale[1];
	matrix[1][3] = 0.0f;

	matrix[2][0] = (cx * sy) * scale[2];
	matrix[2][1] = (-sx) * scale[2];
	matrix[2][2] = (cy * cx) * scale[2];
	matrix[2][3] = 0.0f;

	matrix[3][0] = position[0];
	matrix[3][1] = position[1];
	matrix[3][2] = position[2];
	matrix[3][3] = 1.0f;
}
