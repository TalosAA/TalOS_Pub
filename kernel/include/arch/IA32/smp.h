#ifndef _IA32_SMP_H_
#define _IA32_SMP_H_

#include <libk/stdint.h>
#include <libk/stddef.h>
#include <IA32/tables.h>

void SendInitIPI(uint32_t cpu_id);
void SendInitIPIDeassert(uint32_t cpu_id);
void SendStartupIPI(uint32_t cpu_id, uint32_t vector);
void SendIPI(uint32_t cpu_id, uint32_t vector);
bool IPI_isPending(void);
void StartupAP(uint32_t AP_Num, uint8_t vector);

#endif