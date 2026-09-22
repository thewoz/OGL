#version 330 core
in vec2 TexCoords;
out vec4 outColor;

uniform sampler2D text;
uniform vec3 color;

//
// srgbToLinear - exact sRGB transfer function. The default framebuffer is sRGB
// and the GPU encodes every write (glWindow enables GL_FRAMEBUFFER_SRGB), so a
// color meant to appear on screen exactly as given has to be linearized first.
//
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    // Linearized so the sRGB framebuffer encoding gives back the given color;
    // as a bonus the glyph coverage now blends in linear space.
    outColor = vec4(srgbToLinear(color), 1.0) * sampled;
}
