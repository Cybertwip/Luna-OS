#include <std/stdint.h>

extern "C" uint32_t _start;

// Define the structure for the ELF note
struct ElfNote {
    uint32_t namesz;  // Size of the name field
    uint32_t descsz;  // Size of the descriptor field
    uint32_t type;    // Type of the note
    char name[4];     // Name of the note (null-terminated "Xen")
    uint32_t desc;    // Descriptor data
};

// Use the struct to define the note in the .note.Xen section
__attribute__((section(".note.Xen"), used))
const ElfNote xen_phys32_entry_note = {
    .namesz = 4,  // Length of "Xen" including null terminator
    .descsz = 4,  // The note's descriptor size for a 32-bit address
    .type = 0x12,  // XEN_ELFNOTE_PHYS32_ENTRY from the Xen public headers
    .name = "Xen",
    .desc = (uint32_t)&_start  // Address where the kernel starts in 32-bit mode
};