#include "sys/system.h"
#include "arch/i386/descriptor_tables.h"

// Global Descriptor Table (GDT) and Interrupt Descriptor Table (IDT) definitions.
gdt_entry_t gdt_entries[5];      // Array to hold 5 GDT entries
gdt_ptr_t gdt_ptr;               // Pointer to the GDT structure

idt_entry_t idt_entries[256];    // Array to hold 256 IDT entries
idt_ptr_t idt_ptr;               // Pointer to the IDT structure
interrupt_handler_t interrupt_handlers[256]; // Array of interrupt handler functions

/**
 * @brief Set a GDT entry with specified parameters.
 * 
 * This function sets a specific entry in the Global Descriptor Table (GDT) with the provided base, limit, access, and granularity flags.
 *
 * @param num    Entry number in the GDT
 * @param base   Base address for the segment
 * @param limit  Segment limit
 * @param access Access flags (e.g., read/write, code/data segment)
 * @param gran   Granularity flags (e.g., 4KB page, 32-bit mode)
 */
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

/**
 * @brief Initialize the Global Descriptor Table (GDT).
 *
 * This function sets up the GDT by defining segments for code, data, and user space,
 * and loading it into the CPU.
 */
void init_gdt() {
    // Set GDT pointer size and base
    gdt_ptr.limit = sizeof(gdt_entry_t) * 5 - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;

    // Define GDT entries
    gdt_set_gate(0, 0, 0, 0, 0);                  // Null segment
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);      // Code segment
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);      // Data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);   // User mode code segment
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);   // User mode data segment

    // Load the GDT into the CPU
    gdt_flush((uint32_t)&gdt_ptr);
}

/**
 * @brief Set an entry in the Interrupt Descriptor Table (IDT).
 * 
 * This function sets a specific entry in the IDT for an interrupt service routine (ISR).
 *
 * @param num   Entry number in the IDT
 * @param base  Base address of the ISR
 * @param sel   Kernel segment selector
 * @param flags Flags for the IDT entry (e.g., privilege level)
 */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

/**
 * @brief Initialize the Interrupt Descriptor Table (IDT).
 *
 * This function sets up the IDT by defining interrupt service routines (ISRs) for each interrupt number
 * and configuring the Programmable Interrupt Controller (PIC).
 */
void init_idt() {
    // Zero all interrupt handlers initially.
    memset((uint8_t*)&interrupt_handlers, 0, sizeof(interrupt_handler_t) * 256);

    // Set the IDT pointer size and base
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // Zero the IDT to start with.
    memset((uint8_t*)&idt_entries, 0, sizeof(idt_entry_t) * 256);

    // Remap the IRQ table for the PICs
    outb(0x20, 0x11);  // Initialize master PIC
    outb(0xA0, 0x11);  // Initialize slave PIC
    outb(0x21, 0x20);  // IRQ offset for master PIC
    outb(0xA1, 0x28);  // IRQ offset for slave PIC
    outb(0x21, 0x04);  // Master PIC uses slave PIC
    outb(0xA1, 0x02);  // Slave PIC is on IRQ2
    outb(0x21, 0x01);  // Enable 8086/88 mode
    outb(0xA1, 0x01);  // Enable 8086/88 mode for slave
    outb(0x21, 0x0);   // Unmask IRQs on master PIC
    outb(0xA1, 0x0);   // Unmask IRQs on slave PIC

    // Set each gate in the IDT (ISRs)
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)isr0 + i, 0x08, 0x8E);
    }

    // IRQ handlers
    for (int i = 32; i < 48; i++) {
        idt_set_gate(i, (uint32_t)irq0 + i - 32, 0x08, 0x8E);
    }

    // Tell the CPU about our new IDT.
    idt_flush((uint32_t)&idt_ptr);
}

/**
 * @brief Initialize the descriptor tables (GDT and IDT).
 *
 * This function initializes both the Global Descriptor Table (GDT) and the Interrupt Descriptor Table (IDT).
 */
void init_descriptor_tables() {
    init_gdt();  // Initialize the GDT
    init_idt();  // Initialize the IDT
}

/**
 * @brief Common handler for interrupts.
 * 
 * This function is called whenever an interrupt is triggered. It checks if a handler is registered for the
 * interrupt and calls the appropriate function. If no handler is registered, it triggers a panic.
 *
 * @param regs Pointer to the register state when the interrupt occurred.
 */
void idt_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);  // Call the registered handler
    } else {
        printk("Unhandled interrupt: %d\n", regs->int_no);
        panic("Unhandled interrupt");
    }
}

/**
 * @brief Register an interrupt handler for a specific interrupt number.
 * 
 * This function registers a custom interrupt handler for a specific interrupt number.
 *
 * @param n Interrupt number to associate with the handler.
 * @param h Function pointer to the interrupt handler.
 */
void register_interrupt_handler(uint8_t n, interrupt_handler_t h) {
    interrupt_handlers[n] = h;  // Register the handler for the interrupt
}

/**
 * @brief Common handler for hardware interrupts (IRQs).
 * 
 * This function is called from the assembly interrupt handler stub. It processes
 * the interrupt, sends End of Interrupt (EOI) signals to the Programmable Interrupt Controller (PIC),
 * and invokes the appropriate registered interrupt handler.
 * 
 * @param regs Pointer to the register state when the interrupt occurred.
 */
void irq_handler(registers_t *regs) {
    // Send an End of Interrupt (EOI) signal to the PICs.
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);  // Reset slave PIC
    }
    outb(0x20, 0x20);  // Reset master PIC

    // Check if a handler is registered for this interrupt and call it if it exists.
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handlers[regs->int_no](regs);  // Call the handler
    }
}
