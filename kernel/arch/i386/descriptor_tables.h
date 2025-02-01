#ifdef __cplusplus
extern "C" {
#endif

#ifndef DESCRIPTOR_TABLES_H
#define DESCRIPTOR_TABLES_H

#include "sys/system.h"

// IRQ numbers.
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46    
#define IRQ15 47    

#define IRQ_PRIMARY_IDE     14
#define IRQ_SECONDARY_IDE   15


#define PIC1_DATA  0x21  /* Master PIC data port */
#define PIC2_DATA  0xa1  /* Slave PIC data port */

#define NR_BLKDEV_MAJOR_TYPES   2

#define BLKDEV_RAM_DISK_MAJOR   0
#define BLKDEV_IDE_DISK_MAJOR   1

/* 
 * GDT entry structure.
 * Defines the layout of a single entry in the Global Descriptor Table (GDT).
 */
typedef struct {
    uint16_t limit_low;     // Lower 16 bits of the segment limit.
    uint16_t base_low;      // Lower 16 bits of the base address.
    uint8_t base_middle;    // Middle 8 bits of the base address.
    uint8_t access;         // Access flags: defines segment's type, privilege level, etc.
    uint8_t granularity;    // Granularity and higher 4 bits of the segment limit.
    uint8_t base_high;      // Upper 8 bits of the base address.
} __attribute__((packed)) gdt_entry_t;

/* 
 * GDT pointer structure.
 * Specifies the address and size of the Global Descriptor Table (GDT).
 */
typedef struct {
    uint16_t limit;         // Limit of the GDT (size - 1).
    uint32_t base;          // Base address of the first GDT entry.
} __attribute__((packed)) gdt_ptr_t;

/* 
 * IDT entry structure.
 * Defines the layout of a single entry in the Interrupt Descriptor Table (IDT).
 */
typedef struct {
    uint16_t base_lo;       // Lower 16 bits of the handler function's address.
    uint16_t sel;           // Kernel segment selector (CS).
    uint8_t always0;        // This field is always set to 0.
    uint8_t flags;          // Flags indicating gate type and privilege level.
    uint16_t base_hi;       // Upper 16 bits of the handler function's address.
} __attribute__((packed)) idt_entry_t;

/* 
 * IDT pointer structure.
 * Specifies the address and size of the Interrupt Descriptor Table (IDT).
 */
typedef struct {
    uint16_t limit;         // Limit of the IDT (size - 1).
    uint32_t base;          // Base address of the first IDT entry.
} __attribute__((packed)) idt_ptr_t;

/* 
 * Registers structure.
 * Represents the CPU state during an interrupt.
 */
typedef struct {
    uint32_t ds;            // Data segment selector.
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by `pusha`.
    uint32_t int_no, err_code; // Interrupt number and error code (if applicable).
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor on interrupt.
} registers_t;

/* 
 * Function pointer type for custom interrupt handlers.
 * A function of this type will handle interrupts with a `registers_t` argument.
 */
typedef void (*interrupt_handler_t)(registers_t *);

/* External assembly functions to load the GDT and IDT */
extern void gdt_flush(uint32_t);
extern void idt_flush(uint32_t);

/* ISR (Interrupt Service Routine) declarations for CPU exceptions (0-31) and custom ISR (255). */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr255();

/* IRQ handler function declarations for hardware interrupts (IRQs 0-15). */
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* Initializes the descriptor tables (GDT and IDT). */
void init_descriptor_tables(void);

/* 
 * Registers a custom interrupt handler for a specific interrupt number.
 * 
 * Parameters:
 *     n: The interrupt number (IRQ or exception).
 *     h: The handler function to be registered.
 */
void register_interrupt_handler(uint8_t n, interrupt_handler_t h);

/* 
 * Common handler for hardware interrupts (IRQs).
 * This is called when an IRQ is triggered.
 * 
 * Parameters:
 *     regs: The registers saved during the interrupt.
 */
void irq_handler(registers_t *regs);

void enable_irq_line(int numirq);

#endif /* DESCRIPTOR_TABLES_H */

#ifdef __cplusplus
}
#endif
