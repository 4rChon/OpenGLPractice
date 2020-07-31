#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D texture_normal1;
  sampler2D texture_height1;
};

uniform bool useNormal;
uniform Material material;
uniform vec3 viewPos;

uniform float minLayers;
uniform float maxLayers;
uniform float heightScale;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
  vec3 dp1 = dFdx(p);
  vec3 dp2 = dFdy(p);
  vec2 duv1 = dFdx(uv);
  vec2 duv2 = dFdy(uv);

  vec3 dp2perp = cross(dp2, N);
  vec3 dp1perp = cross(N, dp1);
  vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
  vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

  float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
  return mat3(T * invmax, B * invmax, N);
}

vec2 parallax_mapping(vec2 texCoords, vec3 V_TBN)
{
  float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V_TBN)));
  float layerDepth = 1.0 / numLayers;
  float currentLayerDepth = 0.0;

  vec2 P = V_TBN.xy / V_TBN.z * heightScale;
  vec2 deltaTexCoords = P / numLayers;

  vec2 currentTexCoords = texCoords;
  float currentDepthMapValue = texture(material.texture_height1, currentTexCoords).r;

  while (currentLayerDepth < currentDepthMapValue)
  {
    currentTexCoords -= deltaTexCoords;
    currentDepthMapValue = texture(material.texture_height1, currentTexCoords).r;
    currentLayerDepth += layerDepth;
  }

  vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

  float afterDepth = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = texture(material.texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;

  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);
  return finalTexCoords;
}

vec3 perturb_normal(vec2 texCoords, mat3 TBN)
{
  vec3 map = texture(material.texture_normal1, texCoords).xyz;
  return normalize(TBN * normalize(map * 2.0 - 1.0));
}

void main()
{
  gPosition = FragPos;

  vec3 viewDir = normalize(viewPos - FragPos);
  mat3 TBN = cotangent_frame(normalize(Normal), -viewDir, TexCoords);
  mat3 TBN_T = transpose(TBN);
  vec3 viewDirTBN = normalize((TBN_T * viewPos) - (TBN_T * FragPos));

  vec2 texCoords = parallax_mapping(TexCoords, viewDirTBN);
  if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    discard;

  gNormal = perturb_normal(texCoords, TBN);
  gAlbedoSpec.rgb = texture(material.texture_diffuse1, texCoords).rgb;
  gAlbedoSpec.a = texture(material.texture_specular1, texCoords).r;
}