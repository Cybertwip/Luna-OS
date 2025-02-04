#include <stddef.h>
#include <stdint.h>
#include <malloc.h>




extern "C" void bleskos(dword_t bootloader_passed_value);
extern "C" void run(dword_t bootloader_passed_value);


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

}


class Graphics {
private:
    uint32_t* backBuffer;  // Physical address of backBuffer
public:
    Graphics() {
        // Allocate backBuffer with the correct size and get its physical address
        backBuffer = static_cast<uint32_t*>(malloc(screen_width * screen_height * sizeof(uint32_t)));
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
    uint32_t* getFrontBuffer() { 
        return (uint32_t*)screen_double_buffer_memory_pointer;
    }
    uint32_t getPitch() const { return screen_width; }

    uint32_t getBitsPerPixel() const { return screen_bpp; }

    void swapBuffers() {
        // Copy from backBuffer to video memory
        size_t bufferSizeBytes = getWidth() * getHeight() * (getBitsPerPixel() / 8);
        copy_raw_image_data(backBuffer, 
                            screen_width, 
                            0, 
                            0, 
                            screen_width, 
                            screen_height, 
                            (dword_t)screen_double_buffer_memory_pointer, 
                            screen_width, 
                            0, 
                            0);

        redraw_screen();
    }
};

extern "C" void run(dword_t bootloader_passed_value) {
    // Call main with the magic number and pointer to multiboot_info structure
    bleskos(bootloader_passed_value);

    Graphics gfx;

    for(;;) {
        gfx.clear();
        gfx.swapBuffers();
        wait_for_user_input();
        move_mouse_cursor();
    }
}