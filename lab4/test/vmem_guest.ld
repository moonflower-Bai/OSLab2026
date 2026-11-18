OUTPUT_ARCH(riscv)
ENTRY(_start)

PHDRS {
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS {
    . = 0x80200000;
    .text : {
        KEEP(*(.text.entry))
        *(.text .text.*)
    } :text
    .rodata : { *(.rodata .rodata.* .srodata .srodata.*) } :text

    . = ALIGN(4096);
    .data : { *(.data .data.* .sdata .sdata.*) } :data
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.sbss .sbss.* .bss .bss.* COMMON)
        . = ALIGN(16);
        __bss_end = .;
    } :data
    .stack (NOLOAD) : {
        . = ALIGN(16);
        . += 16384;
        __stack_top = .;
    } :data
}
