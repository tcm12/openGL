#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec4 fPosEye;
//in vec3 color;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform mat3 lightDirMatrix;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float shadow;
float specularStrength = 0.5f;
float shininess = 64.0f;
void computeLightComponents()
{
	//vec4 fPosEye = view * model * vec4(fPosition, 1.0f);		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	//vec3 normalEye = normalize(fNormal);	
	vec3 normalEye = normalize(normalMatrix * fNormal);
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix *lightDir);
	//vec3 lightDirN = normalize(lightDir);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
	
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	//vec3 reflection = reflect(-lightDirN, normalEye);
	//float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}


float computeShadow()
{
	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	// Get closest depth value from light's perspective;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;
	if (normalizedCoords.z > 1.0f)
		currentDepth = 0.0f;
	//float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);
	float bias = 0.005f;
	float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
	// Check whether current frag pos is in shadow
	return shadow;
}

void main() 
{
    computeLightComponents();
    //shadow = computeShadow();
	shadow = 0.0f;
    //modulate with diffuse map
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;
    
    //compute final vertex color
  //  vec3 color = min((ambient + (1.0f - shadow)*diffuse)* texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow)*specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    //vec3 color = min(ambient + diffuse + specular, 1.0f);
    vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    fColor = vec4(color, 1.0f);
}