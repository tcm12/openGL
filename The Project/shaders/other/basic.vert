#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
//out vec3 color;
out vec4 fragPosLightSpace;
out vec4 fPosEye;;
//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 lightSpaceTrMatrix;
uniform mat4 projection;
uniform mat3 normalMatrix;
//lighting
//uniform vec3 lightDir;
//uniform vec3 lightColor;
//uniform vec3 baseColor;

//vec3 ambient;
//float ambientStrength = 0.2f;
//vec3 diffuse;
void main() 
{
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	fPosition = vPosition;
	//fPosition = model * vec4(vPosition, 1.0f);
	fNormal = vNormal;
	fTexCoords = vTexCoords;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	fPosEye = view * model * vec4(fPosition, 1.0f);
}