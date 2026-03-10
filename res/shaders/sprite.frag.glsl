#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

// set 0: Samplers, texture and storage buffers available to the vertex shader
// set 1: Uniform buffers available to the vertex shader
// set 2: Samplers, texture and storage buffers available to the fragment shader
// set 3: Uniform buffers available to the fragment shader
layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main() {
    vec4 texColor = texture(texSampler, fragTexCoord);
    outColor = texColor * fragColor;
}
