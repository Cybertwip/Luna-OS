#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "mm/kmalloc.h"
#include "mm/kmemdetect.h"
#include "mm/paging.h"
#include "mm/heap.h"

#include "vbe.h" 

static uint32_t physBackBuffer; // until we solve identity mapping for paging.

class Graphics {
private:
    static const uint32_t WIDTH = 1024;    // Reduced width
    static const uint32_t HEIGHT = 768;   // Reduced height
    static const uint32_t BPP = 32;
    uint32_t* backBuffer;  // Physical address of backBuffer
public:
    Graphics() {
        init(WIDTH, HEIGHT, BPP);
        // Allocate backBuffer with the correct size and get its physical address
        backBuffer = static_cast<uint32_t*>(kmalloc0_ap(WIDTH * HEIGHT * sizeof(uint32_t), &physBackBuffer));
    }

    ~Graphics() {
    }

    void clear(uint32_t color = 0x0000FF00) {
        for (uint32_t i = 0; i < WIDTH * HEIGHT; ++i) {
            getBackBuffer()[i] = color;
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
    uint32_t* getBackBuffer() { return backBuffer; }
    uint32_t* getFrontBuffer() { 
        return (uint32_t*)0xFD000000;
    }
    uint32_t getPitch() const { return WIDTH; }

    uint32_t getBitsPerPixel() const { return BPP; }

    void swapBuffers() {
        // Copy from backBuffer to video memory
        disable_paging();
        size_t bufferSizeBytes = WIDTH * HEIGHT * (BPP / 8);
        memcpy((void*)getFrontBuffer(), (void*)physBackBuffer, bufferSizeBytes);

        switch_page_directory(kernel_directory);
    }
};