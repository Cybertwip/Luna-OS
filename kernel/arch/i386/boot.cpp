#include "multiboot.h"

// Define the structure for the ELF note for Xen
struct ElfNote {
    uint32_t namesz;  // Size of the name field
    uint32_t descsz;  // Size of the descriptor field
    uint32_t type;    // Type of the note
    char name[4];     // Name of the note (null-terminated "Xen")
    uint32_t desc;    // Descriptor data
};

// Forward declaration of _start as a function
void _start(unsigned long addr);


__attribute__((section(".note.Xen"), used))
const struct ElfNote xen_phys32_entry_note = {
    4,          // namesz
    4,          // descsz
    0x12,       // type (XEN_ELFNOTE_PHYS32_ENTRY)
    "Xen",      // name (as char[4])
    (uint32_t)_start // desc (entry point address)
};

#define MBOOT_PAGE_ALIGN    (1 << 0)
#define MBOOT_MEM_INFO      (1 << 1)
#define MBOOT_HEADER_MAGIC  0x2BADB002
#define MBOOT_HEADER_FLAGS  (MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO)
#define MBOOT_CHECKSUM      (-(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS))

extern char code, bss, end;

const multiboot_header_t mboot_header = {
    MBOOT_HEADER_MAGIC,      // magic
    MBOOT_HEADER_FLAGS,      // flags
    MBOOT_CHECKSUM,          // checksum
    (uint32_t)&mboot_header, // header_addr
    (uint32_t)&code,         // load_addr
    (uint32_t)&bss,          // load_end_addr
    (uint32_t)&end,          // bss_end_addr
    (uint32_t)_start         // entry_addr
};

// Function prototype for main
void main(unsigned long magic, multiboot_info_t* mbd);

// Kernel entry point
void _start(unsigned long addr) {
    // Disable interrupts
    __asm__ __volatile__("cli");

    // Cast the address to a pointer to multiboot_info_t
    multiboot_info_t* mbd = (multiboot_info_t*)addr;

    // Call main with the magic number and pointer to multiboot_info structure
    main(MBOOT_HEADER_MAGIC, mbd);

    // If main returns, enter an infinite loop
    for(;;);
}