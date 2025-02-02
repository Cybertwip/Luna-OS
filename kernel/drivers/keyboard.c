#include "sys/system.h"
#include "arch/i386/descriptor_tables.h"
#include "screen.h"

// Define keyboard ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Key codes for some common keys
#define KEY_ESC 0x01
#define KEY_ENTER 0x1C
#define KEY_BACKSPACE 0x0E

// Buffer for storing keystrokes
#define BUFFER_SIZE 256
static char key_buffer[BUFFER_SIZE];
static int buffer_head = 0;
static int buffer_tail = 0;

static void keyboard_callback(registers_t *regs) {

    panic("X");
    // Read the scancode from the keyboard data port
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Check if key is pressed (not released) by checking the highest bit
    if (scancode & 0x80) {
        // Key release, we might ignore this or handle special cases
        return;
    }

    // Handle some basic keys for demonstration
    switch (scancode) {
        case KEY_ESC:
            screen_write("ESC pressed\n");
            break;
        case KEY_ENTER:
            screen_write("\n");
            break;
        case KEY_BACKSPACE:
            screen_write("\b \b"); // Backspace, delete the last character
            break;
        default:
            // Here you would typically convert scancode to ASCII if needed
            // For simplicity, we'll just print the scancode
            char str[10];
            itoa(scancode, str, 10);
            screen_write(str);
            screen_write(" ");
            
            // Add key to buffer
            if ((buffer_head + 1) % BUFFER_SIZE != buffer_tail) {
                key_buffer[buffer_head] = scancode;
                buffer_head = (buffer_head + 1) % BUFFER_SIZE;
            }
            break;
    }
}

// Function to check if there's any key data in the buffer
char get_key() {
    if (buffer_tail == buffer_head) {
        return 0; // No data
    }
    
    char key = key_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    return key;
}

void init_keyboard() {
    // Register the keyboard interrupt handler for IRQ1 (keyboard interrupt)
    register_interrupt_handler(IRQ1, &keyboard_callback);
    
    // Enable the keyboard IRQ line
    enable_irq_line(1); // IRQ1 for keyboard
    
    // Optionally, you might initialize the keyboard here, like enabling repeat rates
    // This can be done by sending commands through the status and command ports
}

// Initialization function called from your kernel's main setup
void keyboard_init() {
    init_keyboard();
    screen_write("Keyboard driver initialized\n");
}