void bleskos_main_window(void) {
    read_time();

    while (1) {
        // Redraw the main window
        redraw_screen();

        // Wait for user input
        wait_for_user_input();
        move_mouse_cursor();

    }
}