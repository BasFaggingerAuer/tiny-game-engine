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
    float forestFrac = clamp(max(0.0f, 1.0f - 9.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 450.0f)), 0.0f, 1.0f);
    float grassFrac = (1.0f - forestFrac)*clamp(max(0.0f, 1.0f - 7.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 1200.0f)), 0.0f, 1.0f);
    float mudFrac = (1.0f - grassFrac - forestFrac)*clamp(max(0.0f, 1.0f - 1.0f*slope), 0.0f, 1.0f);
    float rockFrac = 1.0f - forestFrac - grassFrac - mudFrac;
    
    colour = vec4(0.0f);
    colour.x = (0.0f*forestFrac + 1.0f*grassFrac + 2.0f*mudFrac + 3.0f*rockFrac)/255.0f;
}
