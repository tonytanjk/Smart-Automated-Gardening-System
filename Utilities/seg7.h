#undef __ARM_FP


#include "mbed.h"

extern void update(unsigned char val[], int size)	; // update the above variable


extern char convert(char outchar); // converts the outchar to 7 segment display pattern