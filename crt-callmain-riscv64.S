/*
  C runtime startup code (crt) for calling main() in riscv64/linux

  Abhijit Dasgupta (MIT license), 2024 February 10

  At start of exec we have:

    sp          -->     argc
    sp + 1      -->     argv[0]
    sp + 2      -->     argv[1]
    ...,
    sp + argc-1 -->     argv[n-1]
    sp + argc   -->     (null)
    sp + argc+1 -->     envp[0]

  So per riscv64 calling ABI, need to call main after putting:

    a0 = argc   (== [sp])
    a1 = argv   (== &argv[0] == &argc + 1 == sp + 8)
    a2 = envp   (== &envp[0] == argv + argc + 1 == sp + 8 + 8*[sp] + 8)

*/

/* From /usr/include/asm-generic/unistd.h (or #include it) */
#define SYS_exit 93

                    /* .global errno is stored in syscall.S */

                    /* Assembler directives */
                    .text
                    .globl  _start      /* Linker default entry point */
                    .globl  Lmain
                    .align  8

/* Default linker symbol for program entry point */
_start:
                    mv fp, sp           /* setup first stack frame */

                    ld a0, 0(sp)        /* a0 has argc */
                    mv a1, sp        
                    addi a1, a1, 8      /* now a1 = &argv[0] */
                    mv t1, a1
countargvloop:
                    mv t0, zero
                    ld t0, 0(t1)
                    beq t0, zero, done
                    addi t1, t1, 8
                    j countargvloop
done:
                    addi t1, t1, 8
                    mv a2, t1           /* now a2 = &envp[0] */

                    call Lmain

_exitloop:
                    /* mv a1, a0 */     /* a0 contains return code from main */
                    li a7, SYS_exit     /* Syscall number for _exit() */
                    ecall
                    j _exitloop         /* Should not happen! */

