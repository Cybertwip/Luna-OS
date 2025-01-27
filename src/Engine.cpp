#include "Engine.h"
#include <luna/fastmem.h>

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
    unsigned int indices[] = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        0, 3, 7, 7, 4, 0, // Left face
        1, 2, 6, 6, 5, 1, // Right face
        0, 1, 5, 5, 4, 0, // Bottom face
        2, 3, 7, 7, 6, 2  // Top face
    };

    for (unsigned int index : indices) {
        model.indices.push_back(index);
    }

    // Triangle type
    model.type = microtess::triangles::indices::TRIANGLES;

    return model;
}

GameEngine::GameEngine() 
    : graphics(), 
      renderer(&graphics), 
      frameCounter(0),
      mCanvas(getWidth(), getHeight()),
      sampler({255, 0, 0, 255}, {255, 0, 0, 255}, 10, 10),
      depth_buffer(getWidth(), getHeight()),
      model(create_cube_3d<number>()) {

    vertex_buffer.reserve(model.vertices.size());
    for (unsigned ix = 0; ix < model.vertices.size(); ++ix) {
        vertex_attributes v{};
        v.point = model.vertices[ix];
        v.uv = model.uvs[ix];
        vertex_buffer.push_back(v);
    }

    // Precompute the projection matrix
    projection_matrix = camera::perspective<number>(
        math::deg_to_rad(65.0f), mCanvas.width(), mCanvas.height(), 1.0f, 150.0f);
}

void GameEngine::run() {
    while (true) {
        uint32_t start = get_clock_ticks(); // Get current clock ticks

        update();
        render();

        uint32_t elapsed = get_clock_ticks() - start; // Calculate elapsed time
        if (elapsed < TARGET_FRAME_TIME_MS) {
            busy_wait(TARGET_FRAME_TIME_MS - elapsed); // Busy-wait to maintain frame rate
        }

        frameCounter++;
    }
}

void GameEngine::update() {
    t += 0.05f; // Increment time for rotation (60 FPS)
}

uint32_t* GameEngine::getBackBuffer() { return graphics.getBackBuffer(); }
uint32_t GameEngine::getWidth() const { return graphics.getWidth(); }
uint32_t GameEngine::getHeight() const { return graphics.getHeight(); }

uint32_t GameEngine::get_clock_ticks() {
    // Replace this with your kernel's clock function
    static uint32_t ticks = 0;
    return ticks++;
}

void GameEngine::busy_wait(uint32_t milliseconds) {
    uint32_t start = get_clock_ticks();
    while (get_clock_ticks() - start < milliseconds) {
        // Do nothing, just wait
    }
}

void GameEngine::render() {
    // Clear the canvas
    mCanvas.clear({0, 0, 255, 255}); // Clear to blue
    depth_buffer.clear(); // Clear depth buffer

    // Setup MVP matrix
    vertex3<number> rotation = {t, t, t}; // Rotate the cube
    vertex3<number> translation = {0, 0, 60}; // Move the cube back
    vertex3<number> scale = {10, 10, 10}; // Scale the cube

    matrix_4x4<number> model_matrix = matrix_4x4<number>::transform(rotation, translation);
    matrix_4x4<number> view = camera::lookAt<number>({0, 0, 70}, {0, 0, 0}, {0, 1, 0});

    matrix_4x4<number> mvp = projection_matrix * view * model_matrix;

    // Configure shader
    shader.matrix = mvp; // Set MVP matrix
    shader.sampler = &sampler; // Set sampler

    // Draw the model
    mCanvas.drawTriangles<blendmode::Normal, porterduff::None<>, true, true, true>(
        shader,                            // Shader
        mCanvas.width(), mCanvas.height(), // Viewport dimensions
        vertex_buffer.data(),              // Vertex buffer
        model.indices.data(),              // Indices
        model.indices.size(),              // Number of indices
        model.type,                        // Triangle type
        microtess::triangles::face_culling::none, // Disable face culling
        &depth_buffer                      // Depth buffer
    );

    // Copy to back buffer
    uint32_t* dst = getBackBuffer();
    const uint32_t* src = reinterpret_cast<const uint32_t*>(mCanvas.pixels());
    const size_t count = getWidth() * getHeight();
    Luna::memcpy(dst, src, count * sizeof(uint32_t));

    // Swap buffers
    graphics.swapBuffers();
}