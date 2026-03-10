#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D sceneTex;

layout(set = 3, binding = 0, std140) uniform CompositeUBO {
    vec2 subPixelOffset;
    float zoom;
    float _pad0;
    float _pad1;
    float _pad2;
    float _pad3;
    float _pad4;
} compositeUbo;

void main() {
    // Apply sub-pixel offset and zoom
    vec2 uv = fragTexCoord / compositeUbo.zoom + compositeUbo.subPixelOffset;
    vec4 sceneColor = texture(sceneTex, uv);
    outColor = sceneColor;
}