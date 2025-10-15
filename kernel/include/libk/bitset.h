#ifndef _LIBK_BITSET_H_
#define _LIBK_BITSET_H_

#define BITSET_INT32       (0xFFFFFFFF)
#define BITSET_INT64       (0xFFFFFFFFFFFFFFFF)

/* Get a word with the i-th bit hitgh */
#define SINGLE_BIT_WORD(i) (1 << (i))

/* AND and compare the input with to_compare */
#define AND_COMPARE(input, to_compare) (((input) & (to_compare)) == (to_compare))

/* Set the i-th bit of input*/
#define BIT_SET(input, i) ((input) |= (1 << (i)))

/* Reset the i-th bit of input */
#define BIT_RESET(input, i) ((input) &= ~(1 << (i)))

/* Check if the i-th bit is set */
#define BIT_IS_SET(input, i) (((input) & (1 << (i))) == (1 << (i)))

/* Extract a field from a register */
#define BIT_FIELD_32(input, start, len)\
                    (((input) >> (start)) & (BITSET_INT32 >> (32 - len)))
#define BIT_FIELD_64(input, start, len)\
                    (((input) >> (start)) & (BITSET_INT64 >> (64 - len)))

/* To little to big endian */
#define UINT32_SWAP_ENDIAN(input)       (((input>>24)&0xff) | \
                                        ((input << 8) & 0xff0000) | \
                                        ((input >> 8) & 0xff00) | \
                                        ((inpu << 24) & 0xff000000)
#ifdef __ORDER_LITTLE_ENDIAN__
  #define UINT32_LITTLE_ENDIAN(input)   (input)
  #define UINT32_BIG_ENDIAN(input)      UINT32_SWAP_ENDIAN(input)
#else
  #define UINT32_LITTLE_ENDIAN(input)   UINT32_SWAP_ENDIAN(input)
  #define UINT32_BIG_ENDIAN(input)      (input)
#endif

#endif