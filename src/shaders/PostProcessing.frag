#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene_color;
uniform sampler2D scene_bloom;
uniform float exposure;

void main()
{
  const float gamma = 2.2;
  vec3 color = texture(scene_color, TexCoords).rgb;
  vec3 bloom = texture(scene_bloom, TexCoords).rgb;
  vec3 result = color + bloom;
  result = vec3(1.0) - exp(-result * exposure);
  result = pow(result, vec3(1.0 / gamma));
  FragColor = vec4(result, 1.0);
}