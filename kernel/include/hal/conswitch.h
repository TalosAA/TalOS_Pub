#ifndef _HAL_CONSWITCH_H_
#define _HAL_CONSWITCH_H_

#include <libk/stdint.h>

extern void SetSysCallsStackAddress(uint32_t CoreID,
                                    void* address,
                                    uint32_t limit);
extern void SetUserStackAddress(void* address,
                                uint32_t limit);
extern void JumpToUserCode (void* StackAddress,
                            uint32_t StackLimig,
                            void* usrCodeAddr);


#endif