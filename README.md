# CSE 410 - Computer Graphics Sessional

This repository contains the implementation of three major offline assignments for the CSE 410 Computer Graphics course. Each assignment focuses on different aspects of computer graphics, from basic OpenGL rendering to advanced ray tracing techniques.

## 📁 Repository Structure

```
CSE410/
├── OFFLINE1-OpenGL/          # OpenGL Fundamentals & 3D Graphics
├── OFFLINE2-Rasterization/   # Software Rasterization Pipeline
└── OFFLINE3-Ray Tracing/     # Ray Tracing with Lighting & Textures
```

## 🎯 Assignment Overview

### 📋 OFFLINE 1: OpenGL Fundamentals
**Location**: `OFFLINE1-OpenGL/2005110/`

**Assignment Tasks**:
- **Task 1 & 3**: 3D Ball Physics Simulation (`2005110_balldemo.cpp`)
- **Task 2**: Real-time Clock Animation (`2005110_clock.cpp`)

**Key Features**:
- **3D Ball Physics Simulation**: 
  - Realistic ball bouncing with gravity (g = -9.8 m/s²)
  - Collision detection with cube boundaries
  - Configurable restitution coefficient (0.75)
  - Ball spin and velocity physics
  - Real-time animation with 60 FPS
- **Interactive Camera Control**: 
  - 6-degree-of-freedom camera movement
  - Mouse-based rotation controls
  - Keyboard-based translation controls
- **3D Scene Rendering**: 
  - Colored cube with checkered floor pattern
  - 10x10 tile checkerboard floor
  - 3D coordinate axes visualization
- **Real-time Clock Animation**:
  - Precise time synchronization with system clock
  - Hour, minute, and second hands with different styles
  - Tick marks for hours and minutes
  - Smooth animation at 60 FPS

**Main Files**:
- `2005110_balldemo.cpp` - Ball physics demo (Tasks 1 & 3)
- `2005110_clock.cpp` - Clock animation (Task 2)
- `run.sh` - Windows compilation script

**Controls (Ball Demo)**:
- **Arrow Keys**: Camera movement (forward/backward/left/right)
- **Mouse**: Camera rotation (pitch/yaw)
- **Space**: Launch ball with initial velocity
- **R**: Reset ball position and velocity
- **T**: Toggle direction arrow visibility
- **1/2/3**: Adjust camera speed
- **4/5/6**: Adjust rotation speed

**Controls (Clock)**:
- **Real-time**: Automatically updates with system time
- **Precise**: Millisecond-level accuracy

### 🖼️ OFFLINE 2: Software Rasterization Pipeline
**Location**: `OFFLINE2-Rasterization/2005110/`

**Assignment Objective**: Implement a complete software graphics pipeline from 3D scene description to 2D rasterized image.

**Key Features**:
- **Complete Graphics Pipeline**: 
  - Model transformation (translation, rotation, scaling)
  - View transformation (camera positioning)
  - Projection transformation (perspective projection)
  - Clipping and rasterization
- **Matrix Operations**: 
  - 4x4 homogeneous transformation matrices
  - Matrix multiplication and inverse operations
  - Rodrigues rotation formula implementation
- **Triangle Rasterization**: 
  - Scan-line triangle filling algorithm
  - Edge intersection calculations
  - Z-buffer for hidden surface removal
- **Stack-based Transformation**: 
  - Push/pop matrix stack operations
  - Hierarchical transformations
- **BMP Image Output**: Generates high-quality bitmap images

**Pipeline Stages**:
1. **Stage 1** (`stage1.txt`): Model transformation - applies scaling, translation, rotation
2. **Stage 2** (`stage2.txt`): View transformation - positions camera and view direction
3. **Stage 3** (`stage3.txt`): Projection transformation - 3D to 2D projection
4. **Stage 4**: Clipping and rasterization - generates final image and z-buffer

**Input Format**:
- `config.txt`: Screen dimensions, projection parameters
- `scene.txt`: Triangle definitions with transformation commands
- **Transformation Commands**:
  - `triangle x1 y1 z1 x2 y2 z2 x3 y3 z3` - Define triangle
  - `push` - Save current transformation matrix
  - `pop` - Restore previous transformation matrix
  - `translate tx ty tz` - Translation transformation
  - `scale sx sy sz` - Scaling transformation
  - `rotate angle ax ay az` - Rotation around axis

**Output Files**:
- `out.bmp` - Final rasterized image
- `z_buffer.txt` - Depth buffer values
- `stage1.txt`, `stage2.txt`, `stage3.txt` - Intermediate pipeline outputs

**Test Cases**: 4 different scene configurations provided in Resources/1-4/

### 🌟 OFFLINE 3: Ray Tracing with Advanced Lighting
**Location**: `OFFLINE3-Ray Tracing/2005110/`

**Assignment Objective**: Implement a complete ray tracing engine with realistic lighting, materials, and textures.

**Key Features**:
- **Ray Tracing Engine**: 
  - Ray generation from camera
  - Ray-object intersection calculations
  - Recursive ray tracing (configurable depth)
- **Multiple Object Types**: 
  - **Spheres**: With configurable radius, center, and materials
  - **Triangles**: 3D triangle primitives with vertex coordinates
  - **General Quadrics**: Complex mathematical surfaces (ellipsoids, cylinders, etc.)
  - **Floor**: Infinite plane with texture support
- **Advanced Lighting System**: 
  - **Point Lights**: Omnidirectional light sources with color
  - **Spot Lights**: Directional light with cutoff angle and direction
  - **Ambient Lighting**: Global illumination component
- **Material Properties**: 
  - Ambient, diffuse, specular coefficients
  - Shininess parameter for specular highlights
  - Recursive reflection coefficient
- **Texture Mapping**: 
  - Support for BMP texture images
  - Checkerboard pattern fallback
  - UV coordinate mapping
- **Real-time Preview**: OpenGL preview with ray-traced image capture

**Scene File Format** (`scene.txt`):
```
[recursion_level]
[pixels]
[object_count]
[objects...]
[point_light_count]
[point_lights...]
[spotlight_count]
[spotlights...]
```

**Object Definitions**:
- **Sphere**: `sphere center_x center_y center_z radius color_r color_g color_b ambient diffuse specular reflection shininess`
- **Triangle**: `triangle x1 y1 z1 x2 y2 z2 x3 y3 z3 color_r color_g color_b ambient diffuse specular reflection shininess`
- **General Quadric**: `general A B C D E F G H I J ref_x ref_y ref_z length width height color_r color_g color_b ambient diffuse specular reflection shininess`

**Lighting Definitions**:
- **Point Light**: `position_x position_y position_z color_r color_g color_b`
- **Spot Light**: `position_x position_y position_z color_r color_g color_b direction_x direction_y direction_z cutoff_angle`

**Controls**:
- **Arrow Keys**: Camera movement (forward/backward/left/right)
- **Mouse**: Camera rotation (pitch/yaw)
- **0-9**: Capture ray-traced images (different angles)
- **T**: Toggle between texture and checkerboard mode
- **R**: Reset camera to initial position
- **+/-**: Adjust recursion level
- **Space**: Toggle real-time preview

**Output**: High-quality ray-traced BMP images with realistic lighting and shadows

## 🚀 Getting Started

### Prerequisites
- **OpenGL**: For real-time rendering (Offline 1 & 3)
- **GLUT/FreeGLUT**: OpenGL Utility Toolkit
- **GLEW**: OpenGL Extension Wrangler Library
- **C++ Compiler**: GCC, MSVC, or compatible compiler
- **Windows**: For the provided run scripts

### Compilation Instructions

#### OFFLINE 1: OpenGL (Windows)
```bash
cd OFFLINE1-OpenGL/2005110/
# For Ball Demo
g++ 2005110_balldemo.cpp -o balldemo.exe -lfreeglut -lglew32 -lopengl32 -lglu32
# For Clock Demo
g++ 2005110_clock.cpp -o clock.exe -lfreeglut -lglew32 -lopengl32 -lglu32
```

#### OFFLINE 2: Rasterization
```bash
cd OFFLINE2-Rasterization/2005110/
g++ -o rasterizer 2005110.cpp
./rasterizer
```

#### OFFLINE 3: Ray Tracing (Windows)
```bash
cd OFFLINE3-Ray Tracing/2005110/
g++ 2005110_main.cpp -o raytracer.exe -lfreeglut -lglew32 -lopengl32 -lglu32
```

### Input Files

#### OFFLINE 2: Scene Configuration
- `scene.txt` - Scene description with triangles and transformations
- `config.txt` - Screen dimensions and projection parameters
- **Format**: `screen_width screen_height z_front z_rear`

#### OFFLINE 3: Scene and Texture Files
- `scene.txt` - Complete scene description with objects, lights, and camera
- `sample_texture.bmp`, `sample_texture2.bmp` - Texture images
- `stb_image.h` - Single-header image loading library

## 🎨 Technical Highlights

### Graphics Pipeline Implementation (Offline 2)
- **Matrix Transformations**: Complete 4x4 homogeneous transformation pipeline
- **Perspective Projection**: Realistic 3D to 2D projection with configurable parameters
- **Clipping Algorithms**: View frustum culling and edge clipping
- **Scan-line Rasterization**: Efficient triangle filling with edge walking
- **Z-Buffer Algorithm**: Accurate hidden surface removal

### Ray Tracing Features (Offline 3)
- **Ray-Object Intersection**: Efficient intersection calculations for all object types
- **Phong Lighting Model**: Realistic illumination with ambient, diffuse, and specular components
- **Recursive Rendering**: Multi-bounce light simulation with configurable depth
- **Texture Sampling**: Bilinear texture interpolation with UV coordinate mapping
- **Shadow Casting**: Accurate shadow computation for all light types

### Performance Optimizations
- **Z-Buffer**: Efficient hidden surface removal with depth testing
- **Bounding Box**: Fast intersection testing for complex objects
- **Early Termination**: Optimized ray traversal with early exit conditions
- **Matrix Stack**: Efficient hierarchical transformations

## 📊 Sample Outputs

Each assignment generates different types of outputs:
- **Offline 1**: Real-time 3D interactive graphics with physics simulation
- **Offline 2**: Static BMP images from 3D scenes with accurate depth information
- **Offline 3**: High-quality ray-traced images with realistic lighting, shadows, and reflections

## 🔧 Customization

### Scene Configuration
- **Offline 2**: Modify `scene.txt` to change triangle positions and transformations
- **Offline 3**: Edit `scene.txt` to adjust object positions, materials, and lighting
- **Camera Parameters**: Adjust view and projection settings for different perspectives
- **Texture Files**: Replace texture images for custom materials

### Rendering Parameters
- **Resolution**: Change screen dimensions for different output quality
- **Lighting**: Modify light positions, colors, and intensities
- **Materials**: Adjust ambient, diffuse, specular, and reflection coefficients
- **Recursion**: Control ray tracing depth for quality vs. performance trade-off

## 📚 Learning Outcomes

This repository demonstrates:
- **3D Graphics Fundamentals**: Understanding of 3D coordinate systems, transformations, and projections
- **Rendering Pipeline**: Complete software graphics pipeline from 3D scene to 2D image
- **Lighting Models**: Implementation of realistic lighting algorithms (Phong model)
- **Ray Tracing**: Advanced rendering techniques for photorealistic images
- **OpenGL Programming**: Real-time graphics programming with interactive controls
- **Software Rasterization**: Understanding of low-level graphics operations
- **Physics Simulation**: Real-time physics with collision detection and response
- **Matrix Mathematics**: 4x4 transformation matrices and homogeneous coordinates

## 👨‍💻 Author

**Student ID**: 2005110  
**Course**: CSE 410 - Computer Graphics Sessional  
**Institution**: Bangladesh University of Engineering and Technology

## 📄 License

This project is created for educational purposes as part of the CSE 410 Computer Graphics course.

---

*This repository showcases a comprehensive understanding of computer graphics concepts, from basic 3D rendering and physics simulation to advanced ray tracing techniques, demonstrating both theoretical knowledge and practical implementation skills across the entire graphics pipeline.* 
