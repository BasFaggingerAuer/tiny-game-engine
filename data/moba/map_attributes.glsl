#version 150

precision highp float;

uniform sampler2D source;
uniform vec2 sourceInverseSize;
uniform float mapScale;

in vec2 tex;
out vec4 colour;

void main(void)
{
    float height = texture(source, tex).x;
    float east = texture(source, tex + vec2(sourceInverseSize.x, 0.0f)).x;
    float west = texture(source, tex - vec2(sourceInverseSize.x, 0.0f)).x;
    float north = texture(source, tex + vec2(0.0f, sourceInverseSize.y)).x;
    float south = texture(source, tex - vec2(0.0f, sourceInverseSize.y)).x;
    
    vec3 normal = normalize(vec3(west - east, mapScale, south - north));
    
    float slope = 1.0f - normal.y;
    float type = 0.0f;
    
    if (slope > 0.1f)
    {
        type = 0.0f;
    }
    else
    {
        type = 2.0f;
    }

    colour = vec4(type/255.0f, 0.0f, 0.0f, 0.0f);
}
