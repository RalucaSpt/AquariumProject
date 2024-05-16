#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;

void main()
{
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 color = texture(diffuseMap, TexCoords).rgb;
    float roughness = texture(roughnessMap, TexCoords).r;

    // Iluminare ambientalã de bazã
    vec3 ambient = 0.1 * color;

    FragColor = vec4(ambient, 1.0);
}
