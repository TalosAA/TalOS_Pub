#ifndef _IA32_TABS_H_
#define _IA32_TABS_H_

#include <IA32/tab_defs.h>
#include <libk/stdint.h>
#include <libk/stddef.h>

/* IA32 GDT Entry */
typedef struct {
  uint16_t    limit_low;          // limit 15:0
  uint16_t    base_low;           // base 15:0
  uint8_t     base_mid;           // base 23:16
  uint8_t     p_dpl_s_type;       // p (1) | dpl (2) | s (1) | type
  uint8_t     g_db_a_limit_high;  // g (1) | db (1) | 0 | a (1) | limit 19:16
  uint8_t     base_high;          // base 31:24
} __is_packed gdt_entry_t;

/* IA32 GDTr entry */
typedef struct {
  uint16_t	limit;
  uint32_t	base;
} __is_packed sGdtr_t;

/* IA32 IDT Entry */
typedef struct {
  uint16_t    isr_low;      // The lower 16 bits of the ISR's address
  uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
  uint8_t     reserved;     // Set to zero
  uint8_t     attributes;   // Type and attributes; see the IDT page
  uint16_t    isr_high;     // The higher 16 bits of the ISR's address
} __is_packed idt_entry_t;

/* IA32 IDTr Entry */
typedef struct {
  uint16_t	limit;
  uint32_t	base;
} __is_packed sIdtr_t;

// A struct describing a Task State Segment.
typedef struct {
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // Unused...
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         // The value to load into ES when we change to kernel mode.
   uint32_t cs;         // The value to load into CS when we change to kernel mode.
   uint32_t ss;         // The value to load into SS when we change to kernel mode.
   uint32_t ds;         // The value to load into DS when we change to kernel mode.
   uint32_t fs;         // The value to load into FS when we change to kernel mode.
   uint32_t gs;         // The value to load into GS when we change to kernel mode.
   uint32_t ldt;        // Unused...
   uint16_t trap;
   uint16_t iomap_base;
} __is_packed tss_entry_t;

void Set_IDT_descriptor(idt_entry_t* inIdt,
                        uint8_t vector,
                        void* isr,
                        uint8_t flags);
void Set_GDT_descriptor(gdt_entry_t* inGdt,
                        uint8_t vector,
                        uint32_t limit,
                        uint32_t base,
                        uint8_t p_dpl_s_type,
                        uint8_t flags);

void Load_GDT(sGdtr_t*);
void Load_IDT(sIdtr_t*);
void Load_TSS(void);


#endif