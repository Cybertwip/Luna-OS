//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

void wait_for_user_input(void) {
    byte_t *keyboard_data_mem = (byte_t *) (usb_keyboard_data_memory);

    // Check for internet status change
    if (internet_last_status != internet.status) {
        internet_last_status = internet.status;
        internet_status_change = STATUS_TRUE;
    } else {
        internet_status_change = STATUS_FALSE;
    }

    // Detect new USB devices
    if (usb_new_device_detected == STATUS_TRUE) {
        detect_usb_devices();
        usb_new_device_detected = STATUS_FALSE;
    }

    // Process PS/2 mouse data if available
    if (ps2_mouse_wait == 0) {
        ps2_mouse_convert_received_data();
        for (int i = 0; i < 4; i++) ps2_mouse_data[i] = 0;
        ps2_mouse_wait = 1;
    }

    // Process USB mouse packet
    if (usb_mouse_packet_received == STATUS_TRUE) {
        usb_mouse_process_received_data();
        usb_mouse_packet_received = STATUS_FALSE;
    }

    // Process USB keyboard packet
    if (usb_keyboard_packet_received == STATUS_TRUE) {
        keyboard_process_code(usb_keyboard_code_of_pressed_key);
        for (int i = 0; i < 8; i++) keyboard_data_mem[i] = 0;
        usb_keyboard_packet_received = STATUS_FALSE;
    }
    // Handle screenshot key
    #ifndef NO_PROGRAMS
    if (keyboard_code_of_pressed_key == KEY_PRINT_SCREEN) {
        copy_memory((dword_t)screen_double_buffer_memory_pointer, 
                    screenshoot_image_info_data_mem, 
                    screenshoot_image_info_data_length);
        show_system_message("Screenshot taken");
        wait(500);
        remove_system_message();
        screenshot_was_made = STATUS_TRUE;
        screenshot_is_cropped = STATUS_FALSE;
        keyboard_code_of_pressed_key = 0;
    }
    #endif

    // Run scheduled tasks
//     scheduler_user_input(); // this causes delay instead
}
