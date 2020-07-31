layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_normal1;
  sampler2D texture_height1;
};

struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 color;
  samplerCube shadowMap;
};

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;


uniform Material material;
#ifdef POINT_LIGHTS
uniform PointLight pointLight[POINT_LIGHTS];
#endif
uniform vec3 viewPos;
uniform float far_plane;
uniform float height_scale;
uniform float min_layers;
uniform float max_layers;
uniform bool height_map;
uniform bool normal_map;


vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

// N - world-space normal
// p - world-space position
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

float shadow_calc(float bias, vec3 lightPos, samplerCube shadowMap)
{
  vec3 fragToLight = fs_in.FragPos - lightPos;
  float currentDepth = length(fragToLight);

  float shadow = 0.0;
  int samples = 20;
  float viewDistance = length(viewPos - fs_in.FragPos);
  float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
  for (int i = 0; i < samples; ++i)
  {
    float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r * far_plane;
    shadow += int(currentDepth - bias > closestDepth);
  }

  shadow /= float(samples);

  return shadow;
}

vec2 parallax_mapping(vec2 texcoords, vec3 V_TBN)
{
  float numLayers = mix(max_layers, min_layers, abs(dot(vec3(0.0, 0.0, 1.0), V_TBN)));
  float layerDepth = 1.0 / numLayers;
  float currentLayerDepth = 0.0;

  vec2 P = V_TBN.xy / V_TBN.z * height_scale;
  vec2 deltaTexCoords = P / numLayers;

  vec2 currentTexCoords = texcoords;
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

vec3 perturb_normal(vec2 texcoord, mat3 TBN)
{
  if (normal_map)
  {
    vec3 map = texture2D(material.texture_normal1, texcoord).xyz;
#ifdef WITH_NORMALMAP_UNSIGNED
    map = map * 255./127. - 128./127.;
#endif
#ifdef WITH_NORMALMAP_2CHANNEL
    map.z = sqrt(1. - dot(map.xy, map.xy));
#endif
#ifdef WITH_NORMALMAP_GREEN_UP
    map.y = -map.y;
#endif
    return normalize(TBN * normalize(map * 2.0 - 1.0));
  }
  
  return normalize(TBN * fs_in.Normal);
}

vec3 calc_point_light(PointLight light, vec3 N, vec3 V, vec2 texcoords)
{
  vec3 light_dir = normalize(light.position - fs_in.FragPos);
  float diffuse = clamp((dot(fs_in.Normal, light_dir)), 0.0, 1.0);
  vec3 halfVector = normalize(light_dir + V);
  float specular = pow(clamp(dot(N, halfVector), 0.0, 1.0), 64.0);
  float distance = length(light.position - fs_in.FragPos);
  float attenuation = 1.0 / (distance * distance);
  float bias = max(0.05 * (1.0 - dot(N, light_dir)), 0.15);
  float shadow = shadow_calc(bias, light.position, light.shadowMap);
  return (1.0 - shadow) * attenuation * (light.color * (diffuse + specular) + light.ambient);
}

void main()
{
  vec3 view_dir = normalize(viewPos - fs_in.FragPos);
  mat3 TBN = cotangent_frame(normalize(fs_in.Normal), -view_dir, fs_in.TexCoords);
  mat3 TBN_T = transpose(TBN);
  vec3 TBN_view_dir = normalize((TBN_T * viewPos) - (TBN_T * fs_in.FragPos));

  vec2 texcoords = fs_in.TexCoords;
  if (height_map)
  {
    texcoords = parallax_mapping(fs_in.TexCoords, TBN_view_dir);
    if (texcoords.x > 1.0 || texcoords.y > 1.0 || texcoords.x < -0.0 || texcoords.y < -0.0)
      discard;
  }

  vec3 normal = perturb_normal(texcoords, TBN);
  
  vec3 result = vec3(0.0);
  #ifdef POINT_LIGHTS
  for (int i = 0; i < POINT_LIGHTS; i++)
  {
    result += calc_point_light(pointLight[i], normal, view_dir, texcoords);
  }
  #endif
  vec3 color = texture(material.texture_diffuse1, texcoords).rgb;
  FragColor = vec4(color * result, 1.0);
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  BrightColor = vec4(FragColor.rgb * float(brightness > 1.0), 1.0);

}