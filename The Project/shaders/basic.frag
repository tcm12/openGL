#version 410 core

in vec4 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 pointLightSource;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.05f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//
float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;
vec3 pAmbient;
vec3 pDiffuse;
vec3 pSpecular;

float computeShadow()
{

	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	float currentDepth = normalizedCoords.z;
	if (normalizedCoords.z > 1.0f)
		currentDepth = 0.0f;

	float bias = 0.005f;
	float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;

	return shadow;
}

float computeFog()
{
    float fogDensity = 0.0005f;
    float fragmentDistance = length(fPosition);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

    return clamp(fogFactor, 0.0f, 1.0f);
}

void computePointLight(vec3 lightPos, vec3 color){
    vec3 cameraPosEye = vec3(0.0f, 0.0f, 0.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);	
    vec3 lightDirN = normalize(lightPos - fPosition.xyz);

    //compute distance to light
    float dist = length(lightPos - fPosition.xyz);
    vec3 viewDirN = normalize (cameraPosEye - fPosition.xyz);
    //compute attenuation
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
    //compute ambient light
    pAmbient = att * ambientStrength * color;
    //compute diffuse light
    pDiffuse = att * max(dot(normalEye, lightDirN), 0.0f) * color;
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), 0.32f);
    pSpecular = att * specularStrength * specCoeff * color;

}

void computeDirLight()
{
    //compute eye space coordinates
    //vec4 fPosEye = view * model * fPosition;
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosition.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
}

void main() 
{
    computeDirLight();
    float shadow = computeShadow();

    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;
    vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

    //point light
    computePointLight(pointLightSource, vec3(1.0f, 0.0f, 1.0f));
    pAmbient *= texture(diffuseTexture, fTexCoords);
    pDiffuse *= texture(diffuseTexture, fTexCoords);
    pSpecular *= texture(specularTexture, fTexCoords);
    vec3 pointColor = min((pAmbient + (1.0f - shadow) * pDiffuse) + (1.0f - shadow) * pSpecular, 1.0f);

    float fogFactor = computeFog();
    vec3 fogColor = vec3(0.5f, 0.5f, 0.5f);
    fColor = vec4(fogColor * (1 - fogFactor) + color * fogFactor + pointColor, 1.0f);
    //fColor = vec4(pointColor, 1.0f);
}
