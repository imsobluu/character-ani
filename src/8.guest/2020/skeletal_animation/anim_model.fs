#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform vec3 fallbackColor;

void main()
{    
    // Get base color
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    float texLuma = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 baseColor = (texLuma < 0.01) ? fallbackColor : texColor.rgb;
    
    // Normalize the normal (interpolation can denormalize it)
    vec3 norm = normalize(Normal);
    
    // Simple directional lighting (top-left)
    vec3 lightDir = normalize(vec3(1.0, 2.0, 1.0));
    vec3 viewDir = normalize(-FragPos); // Simple approximation
    
    // Ambient
    vec3 ambient = 0.3 * baseColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = 0.4 * spec * vec3(1.0);
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
