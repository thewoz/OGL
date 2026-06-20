# Lighting and Materials

OGL uses a single, deliberately simple **Phong** lighting model, shared between
the base solid objects (sphere, ellipse, cuboid) and the imported 3D models.
The goal is predictable, good-looking shading without the complexity of a full
PBR pipeline.

All lighting math is done in **view space**, where the camera sits at the
origin. The vertex shaders transform positions and normals with the
`view * model` matrix; the light vectors are transformed with the same `view`
matrix on the CPU side (see `glLight::setInShader`). This keeps the fragment
shader trivial: the eye direction is simply `normalize(-fragPos)`.

## Lights (`glLight`)

A light carries the classic Phong triple plus a geometry part:

```cpp
ogl::glLight light;
light.setAmbient (glm::vec3(0.2f));
light.setDiffuse (glm::vec3(0.5f));
light.setSpecular(glm::vec3(1.0f));
```

There are two geometric flavors, selected automatically by the shader:

| set on the light          | behavior                                            |
|:--------------------------|:----------------------------------------------------|
| `setDirection(d)`, d ≠ 0  | **directional** light (like the sun), coming from `-d` |
| `setPosition(p)` only     | **point** light located at `p`                      |
| neither                   | **head light**: the light follows the camera        |

The fragment shaders pick the light direction with the same priority order
(direction → position → head light):

```glsl
vec3 lightDir;
if(length(light.direction) > 0.001)      lightDir = normalize(-light.direction);
else if(length(light.position) > 0.001)  lightDir = normalize(light.position - fragPos);
else                                     lightDir = viewDir; // head light
```

Because of the head-light fallback an object is **always** visibly shaded, even
if you never call `setLight()`.

### Setting a light on an object

The base objects and `glModel` expose a convenience setter that takes a
position and a direction at once:

```cpp
ogl::glCuboid cube(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
cube.setLight(/*position*/ glm::vec3(1.0f, 0.0f, 0.0f),
              /*direction*/ glm::vec3(-1.0f));   // directional light

ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");
model.setLight(glm::vec3(1.0f), glm::vec3(-1.0f));
```

Pass a zero direction (`glm::vec3(0.0f)`) if you want a pure point light from
the given position.

## Materials (`glMaterial`)

Materials only apply to **imported models** (the base objects use a single flat
`color` uniform instead). A material is a compact Phong description:

| field            | meaning                                  |
|:-----------------|:-----------------------------------------|
| `ke` emissive    | self-illumination color                  |
| `ka` ambient     | ambient reflectance                      |
| `kd` diffuse     | diffuse (base) color                     |
| `ks` specular    | specular highlight color                 |
| `ns` shininess   | specular exponent                        |
| `d` opacity      | 1 = opaque, 0 = fully transparent        |

Materials are built automatically from the Assimp material of each mesh; you do
not normally construct them by hand.

### Texture maps

A material may carry the following texture maps. When a map is present it
**overrides** the matching flat color in the shader:

`diffuse`, `specular`, `ambient`, `emissive`, `normals`, `opacity`.

Each map is bound to its own texture unit and the matching sampler uniform
(e.g. `material.diffuseTexture`) is pointed at that unit by
`glTexture::setInShader`. The normal map, when present, is interpreted in
tangent space (`normal * 2 - 1`).

### Final color

The model fragment shader combines everything as:

```
lighting = emissive + ambient + diffuse + specular
color    = vec4( gammaCorrect(lighting), opacity )
```

with a standard gamma of `2.2`.

## What was intentionally left out

To keep the engine small and easy to reason about, the following are **not**
implemented:

- **Shadows.** The model shader carries a shadow-mapping path (PCF soft
  shadows), but it is not driven yet: no depth pass is rendered and the
  light-space matrix is not set, so `withShadow` stays `false` and shadows are
  effectively disabled. Base objects have no shadow support at all.
- **Reflection / refraction**, environment maps, and transparency blending
  beyond a simple per-fragment opacity (`d`).
- **Multiple lights.** Each object/model is lit by a single `glLight`.

These are natural future extensions; the current code keeps the material to the
common Phong subset on purpose.

## Troubleshooting

- **Model looks black.** The model's material probably has no diffuse color and
  no diffuse texture (Assimp left `kd` at zero). Provide a textured model or a
  material with a non-zero diffuse color.
- **Wrong/garbled normal mapping.** Make sure each texture map ends up on its
  own texture unit — this is what `glTexture::setInShader` guarantees by setting
  the sampler uniform to the bound unit. (A missing sampler-unit assignment was
  a historical bug that made every sampler read texture unit 0.)
- **No visible shading.** Check the normals: flat-shaded geometry needs correct
  per-face normals, smooth surfaces need per-vertex normals.
