/* kernel.c — point d'entree logique de windOS 0.01.
 *
 * Demarre en mode texte VGA (pas de mode graphique = zero pilote GPU,
 * zero framebuffer a gerer = le noyau tient dans quelques dizaines
 * de Ko). L'interface est volontairement "quasi-ASCII" : un shell,
 * une bannière dessinée en caracteres, et c'est tout.
 */

#include "vga.h"
#include "keyboard.h"
#include "strutil.h"

extern void dos_run(void);
extern void basic_run_shell(void);

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void print_banner(void) {
    vga_set_color(0x0B);
    vga_print(
        "  _      _         _  ____   _____\n"
        " (_)    (_)       | |/ __ \\ / ____|\n"
        "  __      __ _ ___| | |  | | (___\n"
        "  \\ \\ /\\ / /| '_ \\| | |  | |\\___ \\ \n"
        "   \\ V  V / | | | | | |__| |____) |\n"
        "    \\_/\\_/  |_| |_|_|\\____/|_____/\n"
    );
    vga_set_color(0x0A);
    vga_print("\n  windOS 0.01 -- \"legere comme le vent\"\n");
    vga_set_color(0x07);
    vga_print("  Noyau minimal x86, mode texte VGA, zero dependance.\n\n");
}

static void cmd_help(void) {
    vga_print(
        "Commandes :\n"
        "  help     affiche cette aide\n"
        "  ver      version du systeme\n"
        "  cls      efface l'ecran\n"
        "  echo     affiche du texte\n"
        "  color    change la couleur du texte (0-15)\n"
        "  ascii    reaffiche la banniere\n"
        "  dos      lance l'interpreteur facon DOS\n"
        "  basic    lance le mini interpreteur BASIC\n"
        "  reboot   redemarre la machine\n"
        "  halt     arrete le CPU\n"
    );
}

static void cmd_color(const char* arg) {
    if (arg == 0) {
        vga_print("Syntaxe : color <0-15>\n");
        return;
    }
    int v;
    if (str_to_int(arg, &v) && v >= 0 && v <= 15) {
        vga_set_color((uint8_t) v);
        vga_print("Couleur changee.\n");
    } else {
        vga_print("Valeur invalide (attendu 0-15).\n");
    }
}

static void reboot(void) {
    /* Impulsion sur le controleur clavier 8042 : methode standard et
     * minuscule pour redemarrer un PC x86 sans dependre d'ACPI. */
    uint8_t status;
    do {
        __asm__ volatile ("inb $0x64, %0" : "=a"(status));
    } while (status & 0x02);
    outb(0x64, 0xFE);
}

void kernel_main(void) {
    vga_clear();
    print_banner();

    char line[128];
    char* argv[8];

    while (1) {
        vga_set_color(0x0A);
        vga_print("windOS> ");
        vga_set_color(0x07);

        keyboard_read_line(line, sizeof(line));
        int argc = str_tokenize(line, argv, 8);
        if (argc == 0) continue;

        if (str_eq(argv[0], "help")) {
            cmd_help();
        } else if (str_eq(argv[0], "ver")) {
            vga_print("windOS version 0.01\n");
        } else if (str_eq(argv[0], "cls")) {
            vga_clear();
        } else if (str_eq(argv[0], "ascii")) {
            vga_clear();
            print_banner();
        } else if (str_eq(argv[0], "echo")) {
            for (int i = 1; i < argc; i++) {
                vga_print(argv[i]);
                if (i < argc - 1) vga_putchar(' ');
            }
            vga_putchar('\n');
        } else if (str_eq(argv[0], "color")) {
            cmd_color(argc > 1 ? argv[1] : 0);
        } else if (str_eq(argv[0], "dos")) {
            dos_run();
        } else if (str_eq(argv[0], "basic")) {
            basic_run_shell();
        } else if (str_eq(argv[0], "reboot")) {
            reboot();
        } else if (str_eq(argv[0], "halt")) {
            vga_print("Arret du CPU.\n");
            __asm__ volatile ("cli; hlt");
        } else {
            vga_print("Commande inconnue : ");
            vga_print(argv[0]);
            vga_print(" (tape 'help')\n");
        }
    }
}
