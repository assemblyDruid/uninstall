//
// @uAutoGenFileName: vkTriangleShader.h
// @uCommonName:      vkTriangle_frag
// @uShaderType:      FRAGMENT
//

# version 450
#extension GL_ARB_separate_shader_objects : enable


// Specify index of the framebuffer
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;


void main()
{
    outColor = vec4(fragColor, 1.0);
}
