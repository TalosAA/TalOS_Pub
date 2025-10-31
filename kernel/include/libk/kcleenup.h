#ifndef _KER_CLEENUP_H_
#define _KER_CLEENUP_H_

/* Cleenup function interface definition */
typedef void (*__cleenup_func)(void* obj);

#define CLEENUP_FUNCTION_SET(__fun__, __ID__)      __cleenup_func __cleenup##__ID__ = __fun__
#define CLEENUP_LABEL(__label__, __ID__, __obj__)  __cleenupl__##__label__: __cleenup##__ID__(__obj__)
#define CLEENUP_DEFAULT_LABEL()                    __cleenupl__default:
#define CLEENUP(__label__)                         goto __cleenupl__##__label__;

#endif