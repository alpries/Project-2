/*
  Implementation of syscall(2) in linux/riscv64
  Abhijit Dasgupta (MIT license), 2024 February 10
*/

/* Allocate storage for errno */
    .data
    .global errno
errno:
    .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    .text
    .global Lsyscall

Lsyscall:
    mv      t1,a0
    mv      a0,a1
    mv      a1,a2
    mv      a2,a3
    mv      a3,a4
    mv      a4,a5
    mv      a5,a6
    mv      a6,a7
    mv      a7,t1
    ecall
    lui     a5,0xfffff
    bltu    a5,a0,Lsyscall_error
    ret

Lsyscall_error:
    neg     a0,a0
    la      t1, errno
    sw      a0, 0(t1)
    li      a0,-1
    ret

