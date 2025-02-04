//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


// Optimized mouse cursor redraw
void redraw_mouse_cursor(void) {
    redraw_part_of_screen(mouse_cursor_x, mouse_cursor_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGHT);
}

// Optimized mouse cursor movement
void move_mouse_cursor(void) {
    dword_t old_x = mouse_cursor_x, old_y = mouse_cursor_y;

    // Skip if no movement
    if (!mouse_movement_x && !mouse_movement_y) return;

    mouse_movement_x *= 2;
    mouse_movement_y *= 2;
    

    // Handle X movement
    if (mouse_movement_x < 0x80000000) {
        mouse_cursor_x += mouse_movement_x;
        if (mouse_cursor_x >= screen_width) mouse_cursor_x = screen_width - 1;
    } else {
        mouse_cursor_x -= (0xFFFFFFFF - mouse_movement_x) + 1;
        if (mouse_cursor_x >= screen_width) mouse_cursor_x = 0;
    }
    mouse_movement_x = 0;

    // Handle Y movement
    if (mouse_movement_y < 0x80000000) {
        mouse_cursor_y += mouse_movement_y;
        if (mouse_cursor_y >= screen_height) mouse_cursor_y = screen_height - 1;
    } else {
        mouse_cursor_y -= (0xFFFFFFFF - mouse_movement_y) + 1;
        if (mouse_cursor_y >= screen_height) mouse_cursor_y = 0;
    }
    mouse_movement_y = 0;

    // Update drag-and-drop state
    switch (mouse_click_button_state) {
        case MOUSE_NO_DRAG:
            mouse_cursor_x_previous_dnd = mouse_cursor_x_dnd = mouse_cursor_x;
            mouse_cursor_y_previous_dnd = mouse_cursor_y_dnd = mouse_cursor_y;
            break;
        case MOUSE_CLICK:
            mouse_cursor_x_click = mouse_cursor_x;
            mouse_cursor_y_click = mouse_cursor_y;
            mouse_cursor_x_previous_dnd = mouse_cursor_x_dnd = mouse_cursor_x;
            mouse_cursor_y_previous_dnd = mouse_cursor_y_dnd = mouse_cursor_y;
            break;
        case MOUSE_DRAG:
            mouse_cursor_x_previous_dnd = mouse_cursor_x_dnd;
            mouse_cursor_y_previous_dnd = mouse_cursor_y_dnd;
            mouse_cursor_x_dnd = mouse_cursor_x;
            mouse_cursor_y_dnd = mouse_cursor_y;
            break;
    }

    // Redraw affected screen areas
    redraw_part_of_screen(mouse_cursor_x, mouse_cursor_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGHT);
    redraw_part_of_screen(old_x, old_y, MOUSE_CURSOR_WIDTH, MOUSE_CURSOR_HEIGHT);
}

// Optimized mouse zone check
byte_t is_mouse_in_zone(dword_t up, dword_t down, dword_t left, dword_t right) {
    return (mouse_cursor_x >= left && mouse_cursor_x <= right && mouse_cursor_y >= up && mouse_cursor_y <= down) ? STATUS_TRUE : STATUS_FALSE;
}

// Optimized pixel color fetch
dword_t get_mouse_cursor_pixel_color(void) {
    return mouse_cursor_background; // Assuming the first pixel is representative
}