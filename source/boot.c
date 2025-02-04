#include <stddef.h>
#include <stdint.h>

extern void bleskos(dword_t bootloader_passed_value);

extern int main(int argc, char** argv);

void run(dword_t bootloader_passed_value) {
    // Call main with the magic number and pointer to multiboot_info structure
    bleskos(bootloader_passed_value);
    main(0, NULL);
}