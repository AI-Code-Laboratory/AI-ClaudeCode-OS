/* boot.S — point d'entrée de windOS 0.01
 *
 * En-tête Multiboot : permet à GRUB ou à QEMU (-kernel) de charger
 * directement ce noyau, sans bootloader maison à écrire/maintenir.
 * C'est le choix le plus léger et le plus fiable pour un noyau minimal.
 */

.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot, "a"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Pile du noyau : 16 Ko, largement suffisant pour un shell + deux
 * interpréteurs qui ne récursent jamais profondément. */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.type _start, @function
_start:
    mov $stack_top, %esp
    call kernel_main

    /* kernel_main ne doit jamais revenir, mais si ça arrive :
     * on stoppe le CPU proprement plutôt que d'exécuter n'importe quoi. */
    cli
1:  hlt
    jmp 1b

.size _start, . - _start
