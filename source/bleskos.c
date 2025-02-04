//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define NO_PROGRAMS

#include "bleskos.h"
#include "drivers/system/include.h"
#include "drivers/graphic/include.h"
#include "drivers/ps2/include.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/mouse/mouse.h"
#include "drivers/ide/include.h"
#include "drivers/ahci/include.h"
#include "drivers/storage/include.h"
#include "drivers/filesystems/include.h"
#include "drivers/sound/include.h"
#include "drivers/network/include.h"
#include "drivers/usb/include.h"

#include "libraries/basic/include.h"
#include "libraries/logging/logging.h"
#include "libraries/drawing/include.h"
#include "libraries/graphic_user_interface_elements/include.h"
#include "libraries/decoders/include.h"
#include "libraries/checksums/include.h"
#include "libraries/text_formats/include.h"
#include "libraries/graphic_formats/include.h"
#include "libraries/sound_formats/include.h"
#include "libraries/archive_formats/include.h"
#include "libraries/program_interface/include.h"
#include "libraries/file_dialog/include.h"
#include "libraries/binary_programs_interface/include.h"

#include "drivers/system/include.c"
#include "drivers/graphic/include.c"
#include "drivers/ps2/include.c"
#include "drivers/keyboard/keyboard.c"
#include "drivers/mouse/mouse.c"
#include "drivers/ide/include.c"
#include "drivers/ahci/include.c"
#include "drivers/storage/include.c"
#include "drivers/filesystems/include.c"
#include "drivers/sound/include.c"
#include "drivers/network/include.c"
#include "drivers/usb/include.c"

#include "libraries/basic/include.c"
#include "libraries/logging/logging.c"
#include "libraries/drawing/include.c"
#include "libraries/graphic_user_interface_elements/include.c"
#include "libraries/decoders/include.c"
#include "libraries/checksums/include.c"
#include "libraries/text_formats/include.c"
#include "libraries/graphic_formats/include.c"
#include "libraries/sound_formats/include.c"
#include "libraries/archive_formats/include.c"
#include "libraries/program_interface/include.c"
#include "libraries/file_dialog/include.c"
#include "libraries/binary_programs_interface/include.c"
#include "boot.c"

void draw_gradient(int x, int y, int width, int height, dword_t start_color, dword_t end_color);
dword_t interpolate_color(dword_t start_color, dword_t end_color, int step, int total_steps);
void draw_glow(int x, int y, int width, int height, dword_t color, int intensity);

void bleskos(dword_t bootloader_passed_value) {
    boot_options = bootloader_passed_value;

    bleskos_boot_debug_top_screen_color(0xFF0000); //red top of screen
    initalize_memory();

    bleskos_boot_debug_top_screen_color(0x00FF00); //green top of screen
    initalize_logging();
    bleskos_boot_debug_top_screen_color(0x0000FF); //blue top of screen
    log("Blesk Loader 2025 update 2\n\nPress F2 to save System log as TXT file");
    log_starting_memory();

    bleskos_boot_debug_top_screen_color(0xFFFF00); //yellow top of screen
    initalize_scheduler();
    scan_pci();
    set_interrupts();
    set_pit();
    bleskos_boot_debug_top_screen_color(0xFF00FF); //pink top of screen

    initalize_graphic();
    mouse_cursor_x = 0;
    mouse_cursor_y = 0;
    clear_screen(0x001F2F); // Dark blue background
    set_scalable_char_size(64);

    // Draw a gradient background
    draw_gradient(0, 0, screen_width, screen_height, 0x001F2F, 0x003F5F);

    // Draw a glowing logo
    draw_glow(screen_x_center-(64*7/2), screen_y_center-92, 64*7, 64, 0x00FFFF, 20);
    scalable_font_print("Luna OS", screen_x_center-(64*7/2), screen_y_center-92, 0xFFFFFF); // Cyan color

    print_to_message_window("Version 2025 update 2", screen_y_center);
    draw_empty_square(screen_x_center-161, screen_y_center+30, 322, 15, 0x00FFFF);
    number_of_start_screen_messages = 0;
    (*redraw_framebuffer)();

    bleskos_show_message_on_starting_screen("Reading time format...");
    read_time_format();
    bleskos_show_message_on_starting_screen("Reading ACPI tables...");
    read_acpi_tables();
    
    initalize_hpet();
    bleskos_boot_debug_log_message();
    
    bleskos_show_message_on_starting_screen("Initalizing PS/2 controller...");
    initalize_ps2_controller();
    initalize_ps2_keyboard();
    initalize_ps2_mouse();
    bleskos_show_message_on_starting_screen("Initalizing keyboard...");
    initalize_keyboard();
    bleskos_show_message_on_starting_screen("Initalizing mouse...");
    initalize_mouse();
    bleskos_boot_debug_log_message();

    bleskos_show_message_on_starting_screen("Initalizing storage controllers...");
    initalize_list_of_connected_partitions();
    initalize_drivers_of_filesystems();
    initalize_storage_controllers();
    bleskos_boot_debug_log_message();
    
    bleskos_show_message_on_starting_screen("Initalizing sound card...");
    initalize_sound_card();
    bleskos_boot_debug_log_message();

    bleskos_show_message_on_starting_screen("Initalizing network...");
    initalize_network_connection();

    bleskos_show_message_on_starting_screen("Initalizing USB controllers...");
    initalize_usb_controllers();
    bleskos_boot_debug_log_message();

    bleskos_show_message_on_starting_screen("Starting Luna...");
    bleskos_boot_debug_log_message();

    wait(2000);

}

void bleskos_show_message_on_starting_screen(char *string) {
    if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
        return;
    }

    draw_full_square(0, screen_y_center+65, screen_width, 8, 0x001F2F);
    print_to_message_window(string, screen_y_center+65);
    number_of_start_screen_messages++;
    draw_full_square(screen_x_center-160, screen_y_center+31, (320*number_of_start_screen_messages/BLESKOS_NUMBER_OF_START_SCREEN_MESSAGES), 13, 0x0000FF);
    redraw_part_of_screen(0, screen_y_center+31, screen_width, 42);
}

void bleskos_boot_debug_top_screen_color(dword_t color) {
    if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
        dword_t *vesa_lfb_pointer = (dword_t *) (0x3828);
        dword_t *monitor = (dword_t *) (*vesa_lfb_pointer);
        for(dword_t i=0; i<10000; i++) {
            monitor[i] = color;
        }
    }
}

void bleskos_boot_debug_log_message(void) {
    if((boot_options & BOOT_OPTION_DEBUG_MESSAGES)==BOOT_OPTION_DEBUG_MESSAGES) {
        show_log();
        wait(4000);
        skip_logs();
    }
}

// Additional functions for enhanced visuals
void draw_gradient(int x, int y, int width, int height, dword_t start_color, dword_t end_color) {
    for (int i = 0; i < height; i++) {
        dword_t color = interpolate_color(start_color, end_color, i, height);
        draw_full_square(x, y + i, width, 1, color);
    }
}

dword_t interpolate_color(dword_t start_color, dword_t end_color, int step, int total_steps) {
    int start_r = (start_color >> 16) & 0xFF;
    int start_g = (start_color >> 8) & 0xFF;
    int start_b = start_color & 0xFF;

    int end_r = (end_color >> 16) & 0xFF;
    int end_g = (end_color >> 8) & 0xFF;
    int end_b = end_color & 0xFF;

    int r = start_r + (end_r - start_r) * step / total_steps;
    int g = start_g + (end_g - start_g) * step / total_steps;
    int b = start_b + (end_b - start_b) * step / total_steps;

    return (r << 16) | (g << 8) | b;
}

void draw_glow(int x, int y, int width, int height, dword_t color, int intensity) {
    for (int i = 0; i < intensity; i++) {
        dword_t glow_color = (color & 0xFEFEFE) >> 1; // Dim the color
        draw_empty_square(x - i, y - i, width + 2 * i, height + 2 * i, glow_color);
    }
}