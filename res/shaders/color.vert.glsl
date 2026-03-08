#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 fragColor;

// set 0: Samplers, texture and storage buffers available to the vertex shader
// set 1: Uniform buffers available to the vertex shader
// set 2: Samplers, texture and storage buffers available to the fragment shader
// set 3: Uniform buffers available to the fragment shader
layout(set = 1, binding = 0, std140) uniform UBO {
    mat4 viewProjection;
} ubo;

void main() {
    gl_Position = ubo.viewProjection * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
