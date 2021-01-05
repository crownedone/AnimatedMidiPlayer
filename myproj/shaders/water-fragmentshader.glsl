#version 330 core
precision mediump float;

const float pi = 3.14159;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;
uniform mat4 mymodel_matrix;
uniform mat3 mynormal_matrix;

uniform vec3 cameraPos;
uniform float time;

uniform bool enable[8];
uniform float amplitude[8];
uniform float wavelength[8];
uniform float direction[8];
uniform float speed[8];

uniform sampler2D tex;
uniform samplerCube cubetex;

in vec4 myvertex;

out vec4 color;

uniform int totexture;

uniform struct Light
{
	vec4 position;
	vec4 intensity;
	vec3 direction;
	int type;
} Lights[32];
uniform int num_lights;

uniform struct Material
{
	vec4 kd;
	vec4 ks;
	vec4 ka;
	float specular;
} material;


vec3 waveNormal() {
	float dx = 0.0;
	float dy = 0.0;
	for (int i = 0; i < 8; i++) {
		if (enable[i]) {
			float frequency = 2.0*pi / wavelength[i];
			float phase = speed[i] * frequency;
			float d = direction[i] * pi / 180.0;
			vec2 dir = vec2(cos(d), sin(d));
			float theta = dot(dir, myvertex.xz);
			float angle = theta * frequency + time * phase;

			dx += amplitude[i] * dir.y * frequency * cos(angle);
			dy += amplitude[i] * dir.x * frequency * cos(angle);
		}
	}
	vec3 n = vec3(-dx, -dy, 1.0);
	return normalize(n);
}

void main(void) {
	vec3 normal = waveNormal();
	vec3 eye = normalize(cameraPos - myvertex.xyz);

	vec4 _mypos = myview_matrix * mymodel_matrix * myvertex;
	vec3 mypos = (_mypos.xyz) / _mypos.w;
	vec3 eyepos = vec3(0, 0, 0);

	vec3 mypos_to_eyepos = normalize(eye - mypos);
	//vec3 reflection = reflect(-mypos_to_eyepos, normal);

	vec3 reflection = reflect(eye, normal);
	vec2 texPoint = reflection.xy / reflection.z;
	vec2 texCoord = texPoint * 0.5 + 0.5;
	vec3 skyColor = texture2D(tex, texCoord).rgb;

	float cosi = dot(mypos_to_eyepos, normal);
	float sini = 1.0 - cosi;
	float R = 0.1 + 0.7 * sini;



	color = vec4(0, 0, 0, 0);
	vec4 in_kd;
	/*for (int i = 0; i<num_lights; i++)
	{
		if (totexture == 1) {
			in_kd = texture(tex, mytexturecoordinate.st);
		}
		else in_kd = material.kd;

		color += computeColor(Lights[i].position, Lights[i].intensity,
			in_kd, vec4(0.4, 0.4, 0.4, 0), 50,
			mynormal_matrix * mynormal, myview_matrix * mymodel_matrix * myvertex, vec4(0, 0, 0, 1));
	}*/

	color = vec4(skyColor, 0.55);

}