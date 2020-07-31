#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct PointLight {
  vec3 Position;
  vec3 Ambient;
  vec3 Color;
  samplerCube ShadowMap;
  float Linear;
  float Quadratic;
};

const int POINT_LIGHTS = 16;
uniform PointLight pointLight[POINT_LIGHTS];
uniform vec3 viewPos;

void main()
{
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
  float Specular = texture(gAlbedoSpec, TexCoords).a;

  vec3 lighting = vec3(0);
  vec3 viewDir = normalize(viewPos - FragPos);
  for (int i = 0; i < POINT_LIGHTS; i++)
  {
    // ambient
    vec3 ambient = pointLight[i].Ambient * Diffuse;
    // diffuse
    vec3 lightVector = pointLight[i].Position - FragPos;
    vec3 lightDir = normalize(lightVector);
    vec3 diffuse = Diffuse * max(dot(Normal, lightDir), 0.0);
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = Specular * pow(max(dot(Normal, halfwayDir), 0.0), 64.0);
    // attenuation
    float distance = dot(lightVector, lightVector);
    float attenuation = 1.0 / distance;

    lighting += attenuation * (ambient + (diffuse + specular) * pointLight[i].Color);
  }

  FragColor = vec4(lighting, 1.0);
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  BrightColor = vec4(FragColor.rgb * float(brightness > 1.0), 1.0);
}