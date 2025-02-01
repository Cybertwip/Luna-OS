#pragma once

#include <stdint.h>
#include "graphics.h"
#include "renderer.h"

#include <microgl/camera.h>
#include <microgl/z_buffer.h>
#include <microgl/canvas.h>
#include <microgl/bitmaps/bitmap.h>
#include <microgl/pixel_coders/RGB888_PACKED_32.h>
#include <microgl/shaders/sampler_shader.h>
#include <microgl/samplers/checker_board.h>
#include <microgl/math/matrix_4x4.h>
#include <microgl/math/vertex3.h>
#include <microgl/math.h>
#include <microgl/micro-tess/include/micro-tess/dynamic_array.h>

#include <new>

// Define a simple 3D cube model
template<typename number>
struct model_3d {
    dynamic_array<microgl::vertex3<number>> vertices;
    dynamic_array<microgl::vertex2<number>> uvs;
    dynamic_array<unsigned int> indices;
    microtess::triangles::indices type;
};

template<typename number>
model_3d<number> create_cube_3d();

class GameEngine {
private:
    Graphics graphics;
    Renderer renderer;
    uint32_t frameCounter;
    static const uint32_t TARGET_FPS = 60;
    static const uint32_t TARGET_FRAME_TIME_MS = 1000 / TARGET_FPS;

    // Member variables for rendering
    using number = float;
    using Bitmap24 = bitmap<coder::RGB888_PACKED_32>;
    using Canvas24 = canvas<Bitmap24>;
    using Shader = sampler_shader<number, sampling::checker_board<>>;
    using vertex_attributes = Shader::vertex_attributes;

    Canvas24 mCanvas;
    Shader shader;

    microgl::sampling::checker_board<> sampler;
    microgl::z_buffer<14> depth_buffer;
    
    model_3d<number> model;

    dynamic_array<vertex_attributes> vertex_buffer;

    float t = 0.0f; // Time variable for rotation

    matrix_4x4<number> projection_matrix; // Precomputed projection matrix

public:
    GameEngine();
    void run();
    void update();
    void render();

    uint32_t* getBackBuffer();
    uint32_t getWidth() const;
    uint32_t getHeight() const;

private:
    uint32_t get_clock_ticks();
    void busy_wait(uint32_t milliseconds);
};