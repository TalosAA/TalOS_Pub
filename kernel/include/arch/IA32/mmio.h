#ifndef	_SYS_MMIO_H
#define	_SYS_MMIO_H

#include <libk/stdint.h>

/* Utility inline functions to read/write memory mapped io devices */

static __inline uint8_t MMIO_read8 (uintptr_t addr)
{
    return *((volatile uint8_t*)(addr));
}
static __inline uint16_t MMIO_read16 (uintptr_t addr)
{
    return *((volatile uint16_t*)(addr));
}
static __inline uint32_t MMIO_read32 (uintptr_t addr)
{
    return *((volatile uint32_t*)(addr));
}
static __inline uint64_t MMIO_read64 (uintptr_t addr)
{
    return *((volatile uint64_t*)(addr));
}

static __inline void MMIO_write8 (uintptr_t addr, uint8_t value)
{
    (*((volatile uint8_t*)(addr)))=(value);
}
static __inline void MMIO_write16 (uintptr_t addr, uint16_t value)
{
    (*((volatile uint16_t*)(addr)))=(value);
}
static __inline void MMIO_write32 (uintptr_t addr, uint16_t value)
{
    (*((volatile uint16_t*)(addr)))=(value);
}
static __inline void MMIO_write64 (uintptr_t addr, uint64_t value)
{
    (*((volatile uint64_t*)(addr)))=(value);
}

#endif /* _SYS_MMIO_H */
