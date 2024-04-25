#version 330 core
out vec4 fragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

struct SpotLight{
	bool enable;
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float cutOff;	// Inner Cone
	float outerCutOff;	// Outer Cone
};

struct DirLight{
	bool enable;
	vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
	bool enable;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
};

#define NR_POINT_LIGHTS 4

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 viewPos;
uniform Material material;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform PointLight pointLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main(){

	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 norm = normalize(normal);

	vec3 result = vec3(0.f);
	if(dirLight.enable) result += CalcDirLight(dirLight, norm, viewDir);
	if(pointLight.enable) result += CalcPointLight(pointLight, norm, fragPos, viewDir);
	if(spotLight.enable) result += CalcSpotLight(spotLight, norm, viewDir);
	
	fragColor = vec4(result, 1.f);
	//fragColor = vec4(1.f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	// calc diff & spec
	vec3 lightDir = normalize(-light.direction);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(0.f, dot(reflectDir, viewDir)), material.shininess);

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoord));
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoord));

	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	// calc diff & spec
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(0.f, dot(reflectDir, viewDir)), material.shininess);
	// º∆À„æ‡¿ÎÀ•ºı
	float distance = length(light.position - fragPos);
	float attenuation = 1.f / (light.constant + light.linear * distance + light.quadratic * distance * distance);
	
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoord)) * attenuation;
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoord)) * attenuation;
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoord)) * attenuation;

	return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(0.f, dot(normal, lightDir));
	vec3 reflectDir = reflect(-lightDir,normal);
	float spec = pow(max(0.f,  dot(reflectDir, viewDir)), material.shininess);

	// SpotLight + ±ﬂ‘µ»ÌªØ
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.f, 1.f);

	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, texCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, texCoord)) * intensity;
	vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, texCoord)) * intensity;

	return ambient + diffuse + specular;
}