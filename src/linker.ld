/* linker.ld — place le noyau à 1 Mo, zone standard et sûre en x86,
 * hors de portée du chargement à 0 utilisé par les vieux BIOS/DOS. */

ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot : { *(.multiboot) }
    .text      : { *(.text)   }
    .rodata    : { *(.rodata*) }
    .data      : { *(.data)   }
    .bss       : { *(COMMON) *(.bss) }
}
