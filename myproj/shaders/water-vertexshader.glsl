#version 330 core
precision mediump float;

const float pi = 3.14159;

layout(location = 0) in vec4 vertex_modelspace;
layout(location = 1) in vec3 normal_modelspace;
layout(location = 2) in vec2 texturecoordinate_modelspace;


uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;
uniform mat4 mymodel_matrix;
uniform mat3 mynormal_matrix;

uniform int numWaves;
uniform float waterHeight; 
uniform float time;

uniform bool enable[8];
uniform float amplitude[8];
uniform float wavelength[8];
uniform float direction[8];
uniform float speed[8];


out vec4 myvertex;
out vec4 clipSpace;

float wave(int i) {
	float frequency = 2.0*pi / wavelength[i];
	float phase = speed[i] * frequency;
	float d = direction[i] * pi / 180.0;
	vec2 dir = vec2(cos(d), sin(d));
	float theta = dot(dir, vertex_modelspace.xz);
	return amplitude[i] * sin(theta * frequency + time * phase);
}

float bigWaveHeight() {
	float height = 0.0;
	for (int i = 0; i < 4; i++) {
		if (enable[i])
			height = wave(i);
	}
	return height;
}

void main(void) {
	myvertex = vertex_modelspace;
	float height = waterHeight + bigWaveHeight();
	myvertex.y = height;
	//vPos = vec4(vertex_modelspace.x, height, vertex_modelspace.z);
	clipSpace = myprojection_matrix * myview_matrix * mymodel_matrix * myvertex;
	gl_Position = clipSpace;


	//gl_Position = projection * view * vec4(position, height, 1.0);
}