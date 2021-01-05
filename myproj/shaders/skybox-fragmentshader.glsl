#version 330 core

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;
uniform mat4 mymodel_matrix;

in vec4 myvertex;
in vec3 mynormal;
in vec2 mytexturecoordinate;
 
out vec4 color;

uniform sampler2D tex;
uniform samplerCube cubetex;



uniform int totexture;

 
void main (void)
{   

	vec4 _mypos =  mymodel_matrix * myvertex;
	vec3 mypos = (_mypos.xyz) / _mypos.w;

	vec4 texColor = texture(cubetex, mypos);

	color = texColor;


	color.a = 1.0f;
}
