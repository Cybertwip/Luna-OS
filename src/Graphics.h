#pragma once

#include <stdint.h>
#include <std/string.h>

#include "VBE.h" // Assuming VBE.h is in the same directory or an include path

class Graphics {
private:
    static const uint32_t WIDTH = 1024;    // Reduced width
    static const uint32_t HEIGHT = 768;   // Reduced height
    static const uint32_t BPP = 32;
    uint32_t* backBuffer;  // Pointer to the back buffer

public:
    Graphics() {
        init(WIDTH, HEIGHT, BPP);

        // Allocate backBuffer with the correct size (WIDTH * HEIGHT * (BPP / 8))
        backBuffer = new uint32_t[WIDTH * HEIGHT];
    }

    ~Graphics() {
        delete[] backBuffer;
    }

    const uint32_t getWidth() const { return WIDTH; }
    const uint32_t getHeight() const { return HEIGHT; }

    void init(uint32_t width, uint32_t height, uint32_t bpp) {
        vbe_write(0x4, 0x00);        // Disable
        vbe_write(0x1, width);       // XRes
        vbe_write(0x2, height);      // YRes
        vbe_write(0x3, bpp);         // BPP
        vbe_write(0x4, 0x41);        // Enable + LFB
    }

    // Return backBuffer instead of video memory address
    uint32_t* getBackBuffer() { return backBuffer; }
    uint32_t* getFrontBuffer() { return (uint32_t*)0xFD000000; }
    uint32_t getPitch() const { return WIDTH; }

    void swapBuffers() {
        // Copy from backBuffer to video memory
        // Calculate the size of the backBuffer in bytes: WIDTH * HEIGHT * (BPP / 8)
        size_t bufferSizeBytes = WIDTH * HEIGHT * (BPP / 8);
        memcpy((void*)0xFD000000, backBuffer, bufferSizeBytes);
    }
};