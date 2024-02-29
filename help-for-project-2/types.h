/* Typedefs for various sizes in riscv64 - recall RISC is Little Endian */

/* uint8_t (stdint.h) == b/byte (gdb) */
typedef unsigned char   uchar;
typedef unsigned char   uint8;
typedef unsigned char   ubyte;

/* uint16_t (stdint.h) == h/halfword (gdb) */
typedef unsigned short  ushort;
typedef unsigned short  uint16;
typedef unsigned short  le16;

/* uint32_t (stdint.h) == w/word (gdb) */
typedef unsigned int    uint;
typedef unsigned int    uint32;
typedef unsigned int    le32;

/* uint64_t (stdint.h) == g/giant (gdb) */
typedef unsigned long   ulong;
typedef unsigned long   uint64;
typedef unsigned long   ulong64;
typedef unsigned long   le64;

