OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
 . = 0x10000;		/* 64KiB */
 
  .text : {
    *(.text .text.*)
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*)	/* do not need to distinguish this from .rodata */
    . = ALIGN(16);
    *(.rodata .rodata.*)
    . = ALIGN(0x1000);	/* not enough to separate text/rodata from data/bss */
  }

 . = ALIGN (CONSTANT (COMMONPAGESIZE));
 /* 
  The ALIGN directive advances the current memory address to a multiple
  of the provided argument.  The above ALIGN ensures that the gap between
  the .text and .data sections is large enough so that the boundary
  (where .text ends and .data begins) is a multiple COMMONPAGESIZE bytes,
  and so the linker will be able to place the sections into different
  memory LOAD segments.

  Without it there was a single LOAD segment with rwx permissions,
  as the linker combined the .text and .data sections into a single
  LOAD segment; the original ALIGN(0x1000) under .rodata wasn't enough
  for separating the two.

  www.redhat.com/en/blog/linkers-warnings-about-executable-stacks-and-segments
 */

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*)	/* do not need to distinguish this from .data */
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    *(.sbss .sbss.*)	/* do not need to distinguish this from .bss */
    . = ALIGN(16);
    *(.bss .bss.*)
  }

  PROVIDE(end = .);
}

