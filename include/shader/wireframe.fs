#version 330 core

in vec3 fragColor;
out vec4 outColor;

//
// srgbToLinear - exact sRGB transfer function. The default framebuffer is sRGB
// and the GPU encodes every write (glWindow enables GL_FRAMEBUFFER_SRGB), so a
// color meant to appear on screen exactly as given has to be linearized first.
//
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

void main() {
  // Linearized so the sRGB framebuffer encoding gives back the given color.
  outColor = vec4(srgbToLinear(fragColor), 1.0);
}
