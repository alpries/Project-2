OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
 . = 0x10000;       /* 64KiB (16 4KiB pages):  Satisfy linux! */
 
  .text : {             /* Output section */
    *(.text .text.*)    /* Match (in all input files) these input sections */
    . = ALIGN(16);      /* Needed for making a later align effective!!! */
  }

  .rodata : {               /* Output section */
    . = ALIGN(16);
    *(.srodata .srodata.*)  /* Put these input sections into output .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
    . = ALIGN(16);
  }

  /* Avoid merging the writable .data into executable .text.  But if there is
     no .rodata and no ALIGN in .text above, then this align is ineffective:
     The .data still gets merged into .text, in a single rwx segment :(
     See notes below */
  . = ALIGN (CONSTANT (COMMONPAGESIZE));

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*)  /* Put these inputs into output .data */
    . = ALIGN(16);
    *(.data .data.*)
    . = ALIGN(16);
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*)    /* Put these inputs into output .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}

/* 
Notes:

- The ALIGN directive advances the location counter (current memory
  address) to a multiple of the provided argument.  A COMMONPAGESIZE sized
  alignment before the output .data section (after the end of .text and
  .rodata output sections) should prompt the linker to move to a new
  segment, and helps to avoid segments with write+execute permissions.
  See:
  www.redhat.com/en/blog/linkers-warnings-about-executable-stacks-and-segments

- But ld has strange behavior:  Even if such a COMMONPAGESIZE align
  is present after the .text+.rodata and before the .data section,
  the linker ignores it if there is no .rodata section in the input
  object files and there is no align in the .text section.  Adding a
  `. = ALIGN(16);' within the .text section at the end seems to fix
  this issue.

*/
 
