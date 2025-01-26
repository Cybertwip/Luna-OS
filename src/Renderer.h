// Renderer.h
#pragma once
#include "Graphics.h"
#include "GL/GLContext.h"

class Renderer {
private:
    Luna::GLContext glContext; // GLContext member

public:
    Renderer(Graphics* graphics) : glContext(graphics) {}

    // Forwarding functions to GLContext
    void clear(uint32_t mask) { glContext.clear(mask); }
    void matrixMode(uint32_t mode) { glContext.matrixMode(mode); }
    void loadIdentity() { glContext.loadIdentity(); }
    void perspective(float fov, float aspect, float near, float far) {
        glContext.perspective(fov, aspect, near, far);
    }
    void translatef(float x, float y, float z) {
        glContext.translatef(x, y, z);
    }
    void rotatef(float angle, float x, float y, float z) {
        glContext.rotatef(angle, x, y, z);
    }
    void scalef(float x, float y, float z) {
        glContext.scalef(x, y, z);
    }
    void begin(uint32_t mode) { glContext.begin(mode); }
    void end() { glContext.end(); }
    void color3f(float r, float g, float b) { glContext.color3f(r, g, b); }
    void vertex3f(float x, float y, float z) { glContext.vertex3f(x, y, z); }
    void swapBuffers() { glContext.swapBuffers(); }

    void drawLine(const Luna::Vector2& p1, const Luna::Vector2& p2) {
        glContext.drawLine(p1, p2);
    }

};