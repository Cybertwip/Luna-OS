#pragma once
#include "Graphics.h"
#include <math.h>

namespace Luna {

class GLContext {
private:
    Graphics* graphics;
    uint32_t* framebuffer;
    uint32_t width, height, pitch;

public:
    GLContext(Graphics* g) : graphics(g) {
        framebuffer = graphics->getBackBuffer();
        width = graphics->getWidth();
        height = graphics->getHeight();
        pitch = graphics->getPitch();
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
};

} // namespace Luna