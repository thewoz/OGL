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
if you never configure a light.

### Setting the scene light

Objects do not own a light. The light (and the shadow map) live in a
[`glScene`](../include/core/glScene.hpp), which is passed to `render()`:

```cpp
ogl::glScene scene;
scene.setDirectionalLight(glm::vec3(-1.0f));  // or scene.setPointLight(position)
scene.setAmbient (glm::vec3(0.2f));
scene.setDiffuse (glm::vec3(0.5f));
scene.setSpecular(glm::vec3(1.0f));

cube.render(camera, &scene);
model.render(camera, &scene);
```

Calling `render(camera)` without a scene makes the object fall back to a
default head light (and disables shadow sampling).

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

## Shadows

The primary scene light casts shadows through a single 2D shadow map
([`glShadow`](../include/core/glShadow.hpp), owned by `glScene`). Enable them
and draw the casters in a depth-only pass before the color pass:

```cpp
scene.enableShadows(true);
scene.setBounds(center, radius);   // bounding sphere used to frame the light

if(scene.areShadowsEnabled()) {
  scene.beginShadowPass();
  floor.renderDepth(scene.getShadowShader());
  model.renderDepth(scene.getShadowShader());
  scene.endShadowPass();
}

floor.render(camera, &scene);      // the color pass samples the shadow map
```

A directional light uses an orthographic light box; a point light casts
shadows only inside the cone that looks at the scene center (a full
omnidirectional shadow would need a depth cubemap). The shadow test uses 3×3
PCF with a slope-scaled depth bias (see `solid.fs` / `model.fs`).

## What was intentionally left out

To keep the engine small and easy to reason about, the following are **not**
implemented:

- **Reflection / refraction**, environment maps, and transparency blending
  beyond a simple per-fragment opacity (`d`).
- **Multiple lights.** `glScene` stores a vector of lights so the API can grow,
  but the shaders currently use only the primary light (`scene.light(0)`).
- **Omnidirectional point-light shadows** (depth cubemaps); see above.

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
