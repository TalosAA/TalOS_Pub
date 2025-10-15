#include <IA32/tables.h>

void Set_GDT_descriptor(gdt_entry_t* inGdt,
                        uint8_t vector,
                        uint32_t limit,
                        uint32_t base,
                        uint8_t p_dpl_s_type,
                        uint8_t flags) {
    gdt_entry_t* descriptor        = &inGdt[vector];
    descriptor->limit_low          = ((uint32_t)limit)       & 0xFFFF;
    descriptor->base_low           = ((uint32_t)base)        & 0xFFFF;
    descriptor->base_mid           = ((uint32_t)base >> 16)  & 0xFF;
    descriptor->p_dpl_s_type       = p_dpl_s_type;
    descriptor->g_db_a_limit_high  = ((uint32_t)limit >> 16 & 0xF) 
                                   | (flags << 4);
    descriptor->base_high          = ((uint32_t)base >> 24) & 0xFF;
}

void Set_IDT_descriptor(idt_entry_t* inIdt,
                        uint8_t vector,
                        void* isr,
                        uint8_t flags) {
    idt_entry_t* descriptor    = &inIdt[vector];
    descriptor->isr_low        = (uint32_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08;
    descriptor->attributes     = flags;
    descriptor->isr_high       = (uint32_t)isr >> 16;
    descriptor->reserved       = 0;
}