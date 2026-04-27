# Third-Person Skeletal Character Controller

A 3D character controller built with OpenGL featuring skeletal animations, physics-based movement, and an HDR environment skybox.

## Demo

![Demo](./media/character_demo_window.gif)

## Features

- **Skeletal Animation System**: Smooth bone-based character animation with multiple animation states
  - Walk, Run, Jump, and Taunt animations (FBX-based)
  - Dynamic state transitions with smooth blending
- **Third-Person Camera**: Orbiting camera around the character with adjustable pitch and distance
- **Physics & Movement**:
  - Camera-relative WASD movement controls
  - Momentum-based jumping with carry-over from walk/run speed
  - Bounded rectangular playable area with visual borders
- **Advanced Rendering**:
  - Phong lighting model (ambient, diffuse, specular)
  - Screen-space HDR environment mapping
  - Root-motion filtering for natural animation playback
  - Fallback rendering for textureless models
- **Visual Design**:
  - Red/coral character model (0.88, 0.38, 0.38 RGB)
  - Dark ground plane with neon green border
  - Outdoor HDR skybox (Citrus Orchard - Poly Haven)

## Build Requirements

- **CMake** 3.15+
- **Visual Studio 2022** with C++ workload (or MinGW)
- **OpenGL 3.3+** capable graphics card

### Pre-installed Dependencies

The following are included in the repository:
- **GLFW 3** - Window management and input
- **GLAD** - OpenGL loader
- **GLM** - Math library (vectors, matrices)
- **Assimp** - 3D model and animation loading
- **stb_image** - Image loading

## Building & Running

### Windows (Visual Studio)

```bash
.\build.ps1
cd bin
.\skeletal_animation.exe
```

### Linux/Mac (MinGW or Clang)

```bash
cmake -B out -G "MinGW Makefiles"
cmake --build out --target skeletal_animation
./bin/skeletal_animation
```

## Controls

| Input | Action |
|-------|--------|
| **W/A/S/D** | Move forward/left/back/right (camera-relative) |
| **Shift** | Toggle run mode (increases speed) |
| **Space** | Jump (preserves horizontal momentum) |
| **F** | Taunt animation |
| **Mouse** | Orbit camera around character |
| **Scroll Wheel** | Zoom camera in/out |
| **ESC** | Exit |

## Project Structure

```
character-ani/
├── bin/                          # Built executable and shaders
├── media/                        # Recorded demo video
├── src/
│   ├── 8.guest/2020/skeletal_animation/
│   │   ├── skeletal_animation.cpp    # Main application
│   │   ├── anim_model.vs/fs          # Character shader
│   │   ├── skybox.vs/fs              # Environment shader
│   │   └── walk_area.vs/fs           # Ground plane shader
│   ├── glad.c
│   └── stb_image.cpp
├── includes/
│   ├── learnopengl/                  # Custom utilities
│   ├── glm/                          # GLM math library
│   ├── glad/                         # OpenGL bindings
│   ├── GLFW/                         # Window management
│   ├── assimp/                       # Model loading
│   └── KHR/                          # Platform definitions
├── resources/
│   ├── hdr/citrus_orchard_road_puresky_1k.hdr  # Skybox
│   └── test/                         # Animation FBX files
│       ├── Walking.fbx
│       ├── Running.fbx
│       ├── Jump.fbx
│       └── Taunt.fbx
├── lib/                              # Pre-compiled libraries
│   ├── assimp.lib
│   └── glfw3.lib
├── CMakeLists.txt
├── build.ps1                         # Windows build script
└── README.md                         # This file
```

## Animation System

The character uses a state machine with five states:

1. **Idle** - No movement (fallback to walk animation loop)
2. **Walk** - Default movement state (~1.8 units/sec)
3. **Run** - Faster movement when Shift held (~3.3 units/sec)
4. **Action** - Jump state with momentum physics
5. **Taunt** - Special animation triggered by F key

Each state is driven by a separate FBX animation file. Transitions are guarded to prevent overlap (e.g., can't jump while taunting).

## Technical Details

### Root Motion Filtering

The animator automatically zeros the horizontal (X/Z) translation on root and hips bones, preventing the animation from sliding across the ground. Vertical (Y) motion is preserved for jump arcs.

### Jump Momentum

When jumping, the current ground velocity is captured and scaled:
```
jumpMomentum = currentVelocity × 1.45
```

The momentum then decays exponentially over the jump duration with a decay rate of 1.2, creating natural arc deceleration.

### HDR Tone Mapping

The equirectangular HDR map is tone-mapped using Reinhard tone mapping:
```glsl
color = 1 - exp(-color * 1.35);  // Exposure
color = pow(color, 1/2.2);       // Gamma correction
```

## Asset Requirements

### FBX Animation Files

All animation files should be placed in `resources/test/` and named:
- `Walking.fbx` - Neutral walk cycle
- `Running.fbx` - Run cycle (higher speed)
- `Jump.fbx` - Jump arc with landing
- `Taunt.fbx` - Celebration pose

**Recommended Export Settings (Blender/Maya):**
- FBX 2014/2015 binary format
- Bake animation into bones
- Scale: 1.0 (adjust in code if needed)
- Export with Armature

### HDR Skybox

The skybox is a 1k equirectangular HDR image (`citrus_orchard_road_puresky_1k.hdr` - 1.3 MB). Replace with any HDR map from [Poly Haven](https://polyhaven.com/hdris).

## Customization

### Character Color

Edit in `skeletal_animation.cpp`, main render loop:
```cpp
ourShader.setVec3("fallbackColor", glm::vec3(0.88f, 0.38f, 0.38f));  // RGB [0-1]
```

### Ground Area Size

Modify the plane size and movement bounds:
```cpp
const float planeHalfSize = 20.0f;  // Half-width/depth of playable area
```

### Camera Distance & Angle

Adjust default orbit parameters:
```cpp
float cameraDistance = 4.5f;
const float cameraTargetHeight = 1.8f;
float orbitPitch = 30.0f;  // Default pitch angle
```

### Movement Speeds

```cpp
const float walkSpeed = 1.8f;
const float runSpeed = 3.3f;
const float jumpMomentumCarryMultiplier = 1.45f;
const float jumpMomentumDecay = 1.2f;
```

## Performance Notes

- Character model is scaled to 0.01× to match FBX export scale
- Root motion filtering prevents visual artifacts
- Depth testing disabled during skybox render for far-plane coverage
- Supports up to 100 bones per skeleton

## License

This project uses open-source libraries. See individual license files in `includes/`.

**Assets:**
- Character animations: [Mixamo](https://www.mixamo.com/)
- Skybox: [Poly Haven](https://polyhaven.com/) (CC0)
