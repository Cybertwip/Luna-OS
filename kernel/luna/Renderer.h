// Renderer.h
#pragma once
#include "Graphics.h"
#include "GLContext.h"

class Renderer {
private:
    Luna::GLContext glContext; // GLContext member

public:
    Renderer(Graphics* graphics) : glContext(graphics) {}

    void swapBuffers() { glContext.swapBuffers(); }

};