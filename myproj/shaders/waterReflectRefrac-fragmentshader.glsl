#version 330 core

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;
uniform mat4 mymodel_matrix;
uniform mat3 mynormal_matrix;

uniform sampler2D bumptex;
uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudvMap;
uniform sampler2D depthMap;

uniform float moveFactor;
uniform float time;

in vec4 myvertex;
in vec3 mynormal;
in vec4 clipSpace;
in vec2 textureCoords;


uniform struct Light
{
	bool enable;
	vec4 position;
	vec4 intensity;
	vec3 direction;
	int type;
	bool day;
	bool night;
	float lightRadius;

} Lights[32];

uniform struct Material
{
	vec4 kd;
	vec4 ks;
	vec4 ka;
	float specular;
} material;


out vec4 color;


const float waveStrength = 0.02f;
const float fresnelReflectivity = 0.6f;


float calculateFresnel(vec3 _normal, vec4 _mypos, vec4 _eyepos) {

	vec3 normal = normalize(_normal);

	vec3 mypos = myvertex.xyz / myvertex.z;
	vec3 eyepos = _eyepos.xyz / _eyepos.w;

	vec3 viewVector = normalize(mypos - eyepos);

	float refractiveFactor = dot(viewVector, normal);
	
	refractiveFactor = pow(refractiveFactor, fresnelReflectivity);


	return clamp(refractiveFactor, 0.0, 1.0);

}

void main(void) {
	float fresnelFactor = calculateFresnel(mynormal_matrix * mynormal, mymodel_matrix * myvertex, vec4(0, 0, 0, 1));

	vec2 ndc = (clipSpace.xy / clipSpace.w) / 2.0 + 0.5;
	vec2 refractionTextureCoords = vec2(ndc.x, -ndc.y);
	vec2 refflectionTextureCoords = vec2(ndc.x, ndc.y);
	
	vec2 distortedTexCoords = texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg*0.1;
	distortedTexCoords = textureCoords + vec2(distortedTexCoords.x, distortedTexCoords.y + moveFactor);
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * waveStrength;

	vec2 distortedTexture1 = (texture(dudvMap, vec2(textureCoords.x + moveFactor, textureCoords.y)).rg * 2.0 - 1.0)* waveStrength;
	vec2 distortedTexture2 = (texture(dudvMap, vec2(-textureCoords.x + moveFactor, textureCoords.y+ moveFactor)).rg * 2.0 - 1.0)* waveStrength;
	totalDistortion = distortedTexture1 + distortedTexture2;

	refractionTextureCoords += totalDistortion;
	refractionTextureCoords.x = clamp(refractionTextureCoords.x, 0.001, 0.999);
	refractionTextureCoords.y = clamp(refractionTextureCoords.y, -0.999, -0.001);

	refflectionTextureCoords += totalDistortion;
	refflectionTextureCoords = clamp(refflectionTextureCoords, 0.001, 0.999);


	vec4 reflectionColor = texture(reflectionTexture, refractionTextureCoords);
	vec4 refractionColor = texture(refractionTexture, refflectionTextureCoords);
	


	color = mix(reflectionColor, refractionColor, fresnelFactor);
	color = mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.2f);

}