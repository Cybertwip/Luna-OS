#pragma once
#include "Graphics.h"
#include "luna/math.h"
#include "Renderer.h"


class GameEngine {
private:
    Graphics graphics;
    Renderer renderer;
    uint32_t frameCounter;
    float angleY = 0;
    float angleX = 0;
    
    static const uint32_t TARGET_FPS = 60;
    static const uint32_t TARGET_FRAME_TIME = 1000 / TARGET_FPS;

    Luna::Vector3 cubeVertices[8] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };

    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0},
        {4,5}, {5,6}, {6,7}, {7,4},
        {0,4}, {1,5}, {2,6}, {3,7}
    };

public:
    GameEngine() : graphics(), renderer(&graphics), frameCounter(0) {}

    void run() {
        while (true) {
            update();
            render();
            for (volatile uint32_t i = 0; i < TARGET_FRAME_TIME * 1000; ++i) {}
            frameCounter++;
        }
    }

    void update() {
        angleY += 0.03f;  // Increased from 0.02f to 0.05f (2.5x faster)
        angleX += 0.01f;  // Increased from 0.015f to 0.04f (~2.7x faster)
        
        // Keep angles within 0-2π range
        if (angleY > 2 * Luna::PI) angleY -= 2 * Luna::PI;
        if (angleX > 2 * Luna::PI) angleX -= 2 * Luna::PI;
    }
    
    void render() {
        renderer.clear(0x000000FF);

        auto rotY = Luna::Matrix4x4::rotateY(angleY);  // Fixed: Rotate around Y-axis
        auto rotX = Luna::Matrix4x4::rotateX(angleX);  // Fixed: Rotate around X-axis
        
        // Adjust perspective projection parameters
        auto proj = Luna::Matrix4x4::perspective(
            Luna::PI / 2.5f,  // Wider FOV (72 degrees instead of 60)
            static_cast<float>(graphics.getWidth()) / graphics.getHeight(), 
            0.5f,  // Increased near plane
            50.0f  // Reduced far plane
        );

        Luna::Vector2 projected[8];
        for (int i = 0; i < 8; ++i) {
            auto v = Luna::Vector4(cubeVertices[i].x, cubeVertices[i].y, cubeVertices[i].z, 1.0f);
            v = rotY * v;  // Apply Y-axis rotation
            v = rotX * v;  // Apply X-axis rotation
            v.z += 8.0f;   // Increased camera distance from 5.0f to 8.0f
            v = proj * v;  // Apply projection
            
            // Perspective divide with safety check
            if (v.w != 0 && v.w != 1) {
                float invW = 1.0f / v.w;
                v.x *= invW;
                v.y *= invW;
            }
            
            // Adjust viewport scaling
            int screenX = static_cast<int>((v.x * 0.7f + 1) * 0.5f * graphics.getWidth());
            int screenY = static_cast<int>((1 - (v.y * 0.7f + 1) * 0.5f) * graphics.getHeight());
            
            // Clamp values to screen bounds
            screenX = screenX < 0 ? 0 : (screenX >= (int)graphics.getWidth() ? graphics.getWidth() - 1 : screenX);
            screenY = screenY < 0 ? 0 : (screenY >= (int)graphics.getHeight() ? graphics.getHeight() - 1 : screenY);
            
            projected[i] = Luna::Vector2(screenX, screenY);
        }

        // Draw edges
        for (int i = 0; i < 12; ++i) {
            auto& p1 = projected[edges[i][0]];
            auto& p2 = projected[edges[i][1]];
            renderer.drawLine(
                p1,
                p2
            );
        }

        graphics.swapBuffers();
    }
};