#pragma once

#include <stdint.h>
#include "Graphics.h"
#include "Renderer.h"

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

#include "std/new.h"


// Define a simple 3D cube model
template<typename number>
struct model_3d {
    dynamic_array<vertex3<number>> vertices;
    dynamic_array<vertex2<number>> uvs;
    dynamic_array<unsigned int> indices;
    microtess::triangles::indices type;
};

template<typename number>
model_3d<number> create_cube_3d() {
    model_3d<number> model;

    // Vertices
    model.vertices.push_back({-1, -1, -1});
    model.vertices.push_back({1, -1, -1});
    model.vertices.push_back({1, 1, -1});
    model.vertices.push_back({-1, 1, -1});
    model.vertices.push_back({-1, -1, 1});
    model.vertices.push_back({1, -1, 1});
    model.vertices.push_back({1, 1, 1});
    model.vertices.push_back({-1, 1, 1});

    // UVs
    model.uvs.push_back({0, 0});
    model.uvs.push_back({1, 0});
    model.uvs.push_back({1, 1});
    model.uvs.push_back({0, 1});
    model.uvs.push_back({0, 0});
    model.uvs.push_back({1, 0});
    model.uvs.push_back({1, 1});
    model.uvs.push_back({0, 1});

    // Indices
    model.indices.push_back(0); model.indices.push_back(1); model.indices.push_back(2); // Front face
    model.indices.push_back(2); model.indices.push_back(3); model.indices.push_back(0);
    model.indices.push_back(4); model.indices.push_back(5); model.indices.push_back(6); // Back face
    model.indices.push_back(6); model.indices.push_back(7); model.indices.push_back(4);
    model.indices.push_back(0); model.indices.push_back(3); model.indices.push_back(7); // Left face
    model.indices.push_back(7); model.indices.push_back(4); model.indices.push_back(0);
    model.indices.push_back(1); model.indices.push_back(2); model.indices.push_back(6); // Right face
    model.indices.push_back(6); model.indices.push_back(5); model.indices.push_back(1);
    model.indices.push_back(0); model.indices.push_back(1); model.indices.push_back(5); // Bottom face
    model.indices.push_back(5); model.indices.push_back(4); model.indices.push_back(0);
    model.indices.push_back(2); model.indices.push_back(3); model.indices.push_back(7); // Top face
    model.indices.push_back(7); model.indices.push_back(6); model.indices.push_back(2);

    // Triangle type
    model.type = microtess::triangles::indices::TRIANGLES;

    return model;
}

class GameEngine {
private:
    Graphics graphics;
    Renderer renderer;
    uint32_t frameCounter;
    static const uint32_t TARGET_FPS = 30;
    static const uint32_t TARGET_FRAME_TIME = 1000 / TARGET_FPS;

    // Member variables for rendering
    using number = float;
    using Bitmap24 = bitmap<coder::RGB888_PACKED_32>;
    using Canvas24 = canvas<Bitmap24>;
    using Shader = sampler_shader<number, sampling::checker_board<>>;
    using vertex_attributes = Shader::vertex_attributes;

    Canvas24 canvas;
    Shader shader;

    sampling::checker_board<> sampler;
    z_buffer<14> depth_buffer;
    model_3d<number> model;

    dynamic_array<vertex_attributes> vertex_buffer;

    float t = 0.0f; // Time variable for rotation

public:
    GameEngine() 
        : graphics(), 
          renderer(&graphics), 
          frameCounter(0),
          canvas(getWidth(), getHeight()),
          sampler({255, 0, 0, 255}, {255, 0, 0, 255}, 10, 10),
          depth_buffer(getWidth(), getHeight()),
          model(create_cube_3d<number>()) {

            // Convert model vertices to vertex attributes
            vertex_buffer.reserve(model.vertices.size());

            for (unsigned ix = 0; ix < model.vertices.size(); ++ix) {
                vertex_attributes v{};
                v.point = model.vertices[ix];
                v.uv = model.uvs[ix];
                vertex_buffer.push_back(v);
            }

        }

    void run() {
        while(true) {
            update();
            render();
            for(volatile uint32_t i=0; i<TARGET_FRAME_TIME*1000; ++i) {}
            frameCounter++;
        }
    }

    void update() {
        t += 0.032f; // Increment time for rotation
    }
    
    void render() {
        // Clear the canvas and depth buffer
        canvas.clear({0, 0, 255, 255});
        depth_buffer.clear();

        // Setup MVP matrix
        vertex3<number> rotation = {t, t, t}; // Rotate the cube
        vertex3<number> translation = {0, 0, 20}; // Move the cube back
        vertex3<number> scale = {10, 10, 10}; // Scale the cube

        matrix_4x4<number> model_matrix = matrix_4x4<number>::transform(rotation, translation, scale);
        matrix_4x4<number> view = camera::lookAt<number>({0, 0, 70}, {0, 0, 0}, {0, 1, 0});
        matrix_4x4<number> projection = camera::perspective<number>(
            math::deg_to_rad(90.0f), canvas.width(), canvas.height(), 0.1f, 100.0f);

        matrix_4x4<number> mvp = projection * view * model_matrix;

        shader.matrix = mvp;
        shader.sampler = &sampler;

        // Draw the model
        canvas.drawTriangles<blendmode::Normal, porterduff::None<>, true, true, true>(
            shader,                            // Shader
            canvas.width(), canvas.height(),   // Viewport dimensions
            vertex_buffer.data(),              // Vertex buffer
            model.indices.data(),              // Indices
            model.indices.size(),              // Number of indices
            model.type,                        // Triangle type
            microtess::triangles::face_culling::none, // Disable face culling
            &depth_buffer                      // Depth buffer
        );
        // Copy to back buffer
        uint32_t* dst = getBackBuffer();
        const uint32_t* src = reinterpret_cast<const uint32_t*>(canvas.pixels()); // Cast to const uint32_t*
        const size_t count = getWidth() * getHeight();
        for (size_t i = 0; i < count; ++i)
            dst[i] = src[i];
        graphics.swapBuffers();
    }

    uint32_t* getBackBuffer() { return graphics.getBackBuffer(); }
    uint32_t getWidth() const { return graphics.getWidth(); }
    uint32_t getHeight() const { return graphics.getHeight(); }
};