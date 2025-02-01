// renderer.h
#pragma once
#include "graphics.h"
#include "gl_context.h"

class Renderer {
private:
    Luna::GLContext glContext; // GLContext member

public:
    Renderer(Graphics* graphics) : glContext(graphics) {}

    void swapBuffers() { glContext.swapBuffers(); }

};