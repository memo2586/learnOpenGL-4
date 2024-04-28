#version 330 core
out vec4 fragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	sampler2D texture_ambient1;
	float shininess;
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

uniform vec3 viewPos;
uniform samplerCube skybox;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcReflectLight(vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{	
	vec3 result = vec3(0.f);
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 norm = normalize(normal);

	result += CalcDirLight(dirLight, norm, viewDir);
	result = CalcReflectLight(norm, fragPos, viewDir);
	result += CalcPointLight(pointLight, norm, fragPos, viewDir);


	fragColor = vec4(result, 1.f);
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

vec3 CalcReflectLight(vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 R = reflect(-viewDir, normal); 
	vec3 RB = texture(material.texture_ambient1, texCoord).rgb;
	vec3 RC = texture(skybox, R).rgb * RB;
	
	return RC;
}
