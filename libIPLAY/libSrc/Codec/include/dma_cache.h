#define __read_32bit_c0_register(source, sel)               \
({ int __res;                               \
   if (sel == 0)                           \
       __asm__ __volatile__(                   \
           "mfc0\t%0, $"#source "\n\t"         \
           "nop\n\t"               \
           : "=r" (__res));                \
   else                                \
       __asm__ __volatile__(                   \
           ".set\tmips32\n\t"              \
           "mfc0\t%0, $"#source ", " #sel "\n\t"       \
           "nop\n\t"               \
           ".set\tmips0\n\t"               \
           : "=r" (__res));                \
   __res;                              \
})

#define __write_32bit_c0_register(register, sel, value)          \
do {                                    \
   if (sel == 0)                           \
       __asm__ __volatile__(                   \
           "mtc0\t%z0, $"#register "\n\t"          \
           : : "Jr" ((unsigned int)(value)));      \
   else                                \
       __asm__ __volatile__(                   \
           ".set\tmips32\n\t"              \
           "mtc0\t%z0, $"#register ", " #sel "\n\t"    \
           ".set\tmips0"                   \
           : : "Jr" ((unsigned int)(value)));      \
} while (0)
 
#define __read_ulong_c0_register(reg, sel)              \
   ((sizeof(unsigned long) == 4) ?                 \
   (unsigned long) __read_32bit_c0_register(reg, sel) :        \
   (unsigned long) __read_64bit_c0_register(reg, sel))

#define __write_ulong_c0_register(reg, sel, val)            \
do {                                    \
   if (sizeof(unsigned long) == 4)                 \
       __write_32bit_c0_register(reg, sel, val);       \
   else                                \
       __write_64bit_c0_register(reg, sel, val);       \
} while (0)

#define read_c0_xcontext()  __read_ulong_c0_register(20, 0)
#define write_c0_xcontext(val)  __write_ulong_c0_register(20, 0, val)

extern void dma_invalid_dcache(void);
BYTE * InitBitStreamBufferTo();


enum FTYPE
{  
    WAVE_TYPE=0,
    MP3_TYPE=1,
    AAC_TYPE=2,
};

