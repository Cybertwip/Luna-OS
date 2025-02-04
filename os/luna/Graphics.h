#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>



extern "C" {
    extern dword_t screen_width;
    extern word_t screen_height;
    extern dword_t screen_bpp;
    extern byte_t* screen_double_buffer_memory_pointer;
    extern void redraw_screen(void);
    extern void clear_screen(dword_t color);
    extern void copy_raw_image_data(
                         dword_t source_memory, 
                         dword_t source_width, 
                         dword_t source_x, 
                         dword_t source_y, 
                         dword_t image_width, 
                         dword_t image_height, 
                         dword_t dest_memory, 
                         dword_t dest_width, 
                         dword_t dest_x, 
                         dword_t dest_y);

    extern void move_mouse_cursor(void);
    extern void wait_for_user_input(void);

    extern dword_t mouse_cursor_x;
    extern dword_t mouse_cursor_y;
}


class Graphics {
private:
    uint32_t* backBuffer;  // Physical address of backBuffer
public:
    Graphics() {
        // Allocate backBuffer with the correct size and get its physical address
        backBuffer = reinterpret_cast<uint32_t*>(screen_double_buffer_memory_pointer);
    }

    ~Graphics() {
    }

    void clear(uint32_t color = 0x0000FF) {
        for (uint32_t i = 0; i < getWidth() * getHeight(); ++i) {
            getBackBuffer()[i] = color;
        }
    }


    const uint32_t getWidth() const { return screen_width; }
    const uint32_t getHeight() const { return screen_height; }

    // Return backBuffer instead of video memory address
    uint32_t* getBackBuffer() { return backBuffer; }
    uint32_t getPitch() const { return screen_width; }

    uint32_t getBitsPerPixel() const { return screen_bpp; }

    void swapBuffers() {
        redraw_screen();
    }
};