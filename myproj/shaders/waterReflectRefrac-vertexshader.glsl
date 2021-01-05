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

uniform float waterHeight;

out vec4 myvertex;
out vec3 mynormal;
out vec4 clipSpace;
out vec2 textureCoords;

const float tiling = 6.0f;


void main(void) {
	myvertex = vertex_modelspace;
	myvertex.y += waterHeight;

	mynormal = normal_modelspace;
	textureCoords = texturecoordinate_modelspace * tiling;


	clipSpace = myprojection_matrix * myview_matrix * mymodel_matrix * myvertex;
	gl_Position = clipSpace;
	
}