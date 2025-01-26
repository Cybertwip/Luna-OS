#pragma once
#include "Graphics.h"
#include "luna/math.h"

namespace Luna {

class GLContext {
private:
    Graphics* graphics;
    uint32_t* framebuffer;
    uint32_t width, height, pitch;

    // Matrix stack implementation
    enum { MAX_MATRIX_STACK_DEPTH = 8 };
    Luna::Matrix4x4 projectionStack[MAX_MATRIX_STACK_DEPTH];
    Luna::Matrix4x4 modelviewStack[MAX_MATRIX_STACK_DEPTH];
    int projectionDepth = 0;
    int modelviewDepth = 0;

    // Current state
    Luna::Matrix4x4* currentProjection = &projectionStack[0];
    Luna::Matrix4x4* currentModelview = &modelviewStack[0];
    uint32_t currentColor = 0x00FF00FF; // Default color (magenta)

    // Vertex buffer
    enum { MAX_VERTICES = 64 };
    Luna::Vector4 vertices[MAX_VERTICES];  // Changed from Vector2 to Vector4 to keep track of Z/W
    uint32_t vertexCount = 0;
    uint32_t drawMode = 0;

public:
    GLContext(Graphics* g) : graphics(g) {
        framebuffer = graphics->getBackBuffer();
        width = graphics->getWidth();
        height = graphics->getHeight();
        pitch = graphics->getPitch();
        // Initialize identity matrices
        loadIdentity();
    }

    // Core rendering functions
    void clear(uint32_t color) {
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                framebuffer[y * pitch + x] = color;
            }
        }
    }

    void swapBuffers() {
        graphics->swapBuffers();
    }

    // OpenGL-style API
    void matrixMode(uint32_t mode) {
        if (mode == 0x1701) { // GL_PROJECTION
            currentProjection = &projectionStack[projectionDepth];
        } else if (mode == 0x1700) { // GL_MODELVIEW
            currentModelview = &modelviewStack[modelviewDepth];
        }
    }

    void loadIdentity() {
        *currentProjection = Luna::Matrix4x4::identity();
        *currentModelview = Luna::Matrix4x4::identity();
    }

    void perspective(float fov, float aspect, float near, float far) {
        *currentProjection = Luna::Matrix4x4::perspective(fov * Luna::DEG_TO_RAD, aspect, near, far);
    }

    void translatef(float x, float y, float z) {
        // Ensure translation is applied correctly
        Luna::Matrix4x4 translation = Luna::Matrix4x4::translate(x, y, z);
        *currentModelview = Luna::Matrix4x4::multiply(*currentModelview, translation);
    }

    void scalef(float x, float y, float z) {
        Luna::Matrix4x4 scale = Luna::Matrix4x4::scale(x, y, z);
        *currentModelview = Luna::Matrix4x4::multiply(*currentModelview, scale);
    }

    void rotatef(float angle, float x, float y, float z) {
        float radAngle = angle * Luna::DEG_TO_RAD;
        Luna::Matrix4x4 rotation = Luna::Matrix4x4::identity();
        
        if (x != 0.0f) {
            rotation = Luna::Matrix4x4::multiply(rotation, Luna::Matrix4x4::rotateX(radAngle));
        }
        if (y != 0.0f) {
            rotation = Luna::Matrix4x4::multiply(rotation, Luna::Matrix4x4::rotateY(radAngle));
        }
        if (z != 0.0f) {
            rotation = Luna::Matrix4x4::multiply(rotation, Luna::Matrix4x4::rotateZ(radAngle));
        }
        // Post-multiply the rotation matrix for correct order
        *currentModelview = Luna::Matrix4x4::multiply(*currentModelview, rotation);
    }

    void begin(uint32_t mode) {
        drawMode = mode;
        vertexCount = 0;
    }

    void end() {
        if (drawMode == 0x0001) { // GL_LINES
            for (uint32_t i = 0; i + 1 < vertexCount; i += 2) {
                Luna::Vector2 p1 = clipToScreen(vertices[i]);
                Luna::Vector2 p2 = clipToScreen(vertices[i+1]);
                if(p1.x != -1 && p1.y != -1 && p2.x != -1 && p2.y != -1)  // Check if points are not behind camera
                    drawLineInternal(p1, p2);
            }
        }
        vertexCount = 0;
    }

    void color3f(float r, float g, float b) {
        r = Luna::abs(r); g = Luna::abs(g); b = Luna::abs(b);
        currentColor = (0xFF << 24) | // Alpha (fully opaque)
                       ((static_cast<uint8_t>(b * 255) << 16) & 0xFF0000) |
                       ((static_cast<uint8_t>(g * 255) << 8) & 0x00FF00) |
                       (static_cast<uint8_t>(r * 255) & 0x0000FF);
    }

    void vertex3f(float x, float y, float z) {
        if (vertexCount < MAX_VERTICES) {
            vertices[vertexCount++] = transformVertex({x, y, z, 1.0f});
        }
    }

    void drawLine(const Luna::Vector2& p1, const Luna::Vector2& p2) {
        drawLineInternal(p1, p2);
    }

private:
    Luna::Vector4 transformVertex(const Luna::Vector4& v) {
        Luna::Matrix4x4 mvp = Luna::Matrix4x4::multiply(*currentProjection, *currentModelview);
        return mvp * v;
    }

    Luna::Vector2 clipToScreen(const Luna::Vector4& v) {
        if (v.w <= 0) return { -1, -1 }; // Skip vertices behind camera

        float invW = 1.0f / v.w;
        float x = (v.x * invW + 1) * 0.5f * width;
        float y = (1 - (v.y * invW + 1) * 0.5f) * height;

        x = Luna::clamp(x, 0.0f, static_cast<float>(width - 1));
        y = Luna::clamp(y, 0.0f, static_cast<float>(height - 1));

        return { static_cast<int>(x), static_cast<int>(y) };
    }

    void drawLineInternal(const Luna::Vector2& p1, const Luna::Vector2& p2) {
        int x0 = p1.x, y0 = p1.y;
        int x1 = p2.x, y1 = p2.y;

        int dx = Luna::abs(x1 - x0);
        int sx = x0 < x1 ? 1 : -1;
        int dy = -Luna::abs(y1 - y0);
        int sy = y0 < y1 ? 1 : -1;
        int error = dx + dy;

        while (true) {
            if (x0 >= 0 && x0 < static_cast<int>(width) && y0 >= 0 && y0 < static_cast<int>(height)) {
                framebuffer[y0 * pitch + x0] = currentColor;
            }
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * error;
            if (e2 >= dy) { error += dy; x0 += sx; }
            if (e2 <= dx) { error += dx; y0 += sy; }
        }
    }
};

} // namespace Luna