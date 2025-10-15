#include <IA32/exceptions.h>
#include <libk/stdio.h>

void ex_PageFault(uint32_t id,
                  uint32_t err,
                  uint32_t eip,
                  uint32_t cs,
                  uint32_t eflags)
{
  printf("Page Fault - vect 0x%x; "
         "Error Code 0x%x; "
         "Eip 0x%x; "
         "CS 0x%x; "
         "EFLAGS 0x%x\n",
  id, err, eip, cs, eflags);
}

void ex_DoubleFault(uint32_t id,
                  uint32_t err,
                  uint32_t eip,
                  uint32_t cs,
                  uint32_t eflags)
{
  printf("Double Fault - vect 0x%x; "
         "Error Code 0x%x; "
         "Eip 0x%x; "
         "CS 0x%x; "
         "EFLAGS 0x%x\n",
  id, err, eip, cs, eflags);
}

