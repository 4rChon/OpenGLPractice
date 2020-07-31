#version 330 core
#define NR_POINT_LIGHTS 4
struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_diffuse2;
  sampler2D texture_diffuse3;
  sampler2D texture_diffuse4;
  sampler2D texture_specular1;
  sampler2D texture_specular2;
  sampler2D texture_specular3;
  sampler2D texture_specular4;
  sampler2D texture_normal1;
  sampler2D texture_normal2;
  sampler2D texture_normal3;
  sampler2D texture_normal4;
  sampler2D texture_height1;
  sampler2D texture_height2;
  sampler2D texture_height3;
  sampler2D texture_height4;
};

struct PointLight {
  vec3 position;
  float constant;
  float linear;
  float quadratic;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct DirLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
  float cutOff;
  float outerCutOff;
};

out vec4 FragColor;

uniform Material material;
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

vec3 CalcSpecularTotal();
vec3 CalcDiffuseTotal();
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  vec3 result = CalcDirLight(dirLight, norm, viewDir);
  for (int i = 0; i < NR_POINT_LIGHTS; i++)
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
  FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.direction);
  // diffuse light
  float diff = max(dot(normal, lightDir), 0.0);
  //specular light
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);

  vec3 total_diffuse = CalcDiffuseTotal();
  vec3 ambient = light.ambient * total_diffuse;
  vec3 diffuse = diff * light.diffuse * total_diffuse;
  vec3 specular = spec * light.specular * CalcSpecularTotal();

  return ambient + diffuse + specular;
}

// Point Light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  // diffuse light
  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);

  //specular light
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
  // attenuation
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  vec3 total_diffuse = CalcDiffuseTotal();
  vec3 ambient = light.ambient * total_diffuse;
  vec3 diffuse = diff * light.diffuse * total_diffuse;
  vec3 specular = spec * light.specular * CalcSpecularTotal();

  return attenuation * (ambient + diffuse + specular);
}

// Spot Light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.position - fragPos);
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  // diffuse light
  float diff = max(dot(normal, lightDir), 0.0);

  //specular light
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);

  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  vec3 total_diffuse = CalcDiffuseTotal();
  vec3 ambient = light.ambient * total_diffuse;
  vec3 diffuse = diff * light.diffuse * total_diffuse;
  vec3 specular = spec * light.specular * CalcSpecularTotal();

  return intensity * attenuation * (diffuse + specular) + ambient;
}

vec3 CalcSpecularTotal()
{
  vec3 specular = texture(material.texture_specular1, TexCoords).rgb;
  specular += texture(material.texture_specular2, TexCoords).rgb;
  specular += texture(material.texture_specular3, TexCoords).rgb;
  specular += texture(material.texture_specular4, TexCoords).rgb;
  return specular;
}
vec3 CalcDiffuseTotal()
{
  vec3 diffuse = texture(material.texture_diffuse1, TexCoords).rgb;
  diffuse += texture(material.texture_diffuse2, TexCoords).rgb;
  diffuse += texture(material.texture_diffuse3, TexCoords).rgb;
  diffuse += texture(material.texture_diffuse4, TexCoords).rgb;
  return diffuse;
}