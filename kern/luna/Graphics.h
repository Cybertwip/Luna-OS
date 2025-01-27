#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern "C" {
    #include "mm/kmemory.h"
}

#include "VBE.h" 

class Graphics {
private:
    static const uint32_t WIDTH = 1024;    // Reduced width
    static const uint32_t HEIGHT = 768;   // Reduced height
    static const uint32_t BPP = 32;
    uint32_t backBufferPhys;  // Physical address of backBuffer
    uint32_t* backBuffer;  // Physical address of backBuffer

public:
    Graphics() {
        init(WIDTH, HEIGHT, BPP);

        // Allocate backBuffer with the correct size and get its physical address
        backBuffer = static_cast<uint32_t*>(kalloc_align_phys(WIDTH * HEIGHT * sizeof(uint32_t), &backBufferPhys));
    }

    ~Graphics() {
        kfree(backBuffer); // Use kfree since we used kalloc to allocate backBuffer
    }

    void clear(uint32_t color = 0xFF00FF00) {
        // Fill the back buffer with the specified color
        for (uint32_t i = 0; i < WIDTH * HEIGHT; ++i) {
            backBuffer[i] = color;
        }
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
    uint32_t* getBackBuffer() { return backBufferPhys; }
    uint32_t* getFrontBuffer() { return (uint32_t*)0xFD000000; }
    uint32_t getPitch() const { return WIDTH; }

    void swapBuffers() {
        // Copy from backBuffer to video memory
        // Calculate the size of the backBuffer in bytes: WIDTH * HEIGHT * (BPP / 8)
        size_t bufferSizeBytes = WIDTH * HEIGHT * (BPP / 8);
        memcpy((void*)0xFD000000, backBufferPhys, bufferSizeBytes);
    }
};