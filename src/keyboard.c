/* keyboard.c — pilote clavier PS/2 minimal, par scrutation (polling).
 * Pas d'IDT/IRQ ici volontairement : ça garde le noyau minuscule et
 * évite toute la mécanique d'interruptions pour un shell qui n'a
 * besoin que de lire des touches une par une. */

#include "vga.h"
#include "keyboard.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

static bool_t shift_held = FALSE;

/* Table scancode set 1 -> ASCII (disposition US, sans les touches mortes). */
static const char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0,
};

static const char scancode_to_ascii_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0, 'A','S','D','F','G','H','J','K','L',':','"','~',
    0, '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0, ' ', 0,
};

#define SCANCODE_LSHIFT      0x2A
#define SCANCODE_RSHIFT      0x36
#define SCANCODE_LSHIFT_REL  0xAA
#define SCANCODE_RSHIFT_REL  0xB6

/* Bloque jusqu'à ce qu'une touche produise un caractère, puis le renvoie. */
char keyboard_getchar(void) {
    while (1) {
        if (!(inb(KBD_STATUS_PORT) & 1)) continue;

        uint8_t code = inb(KBD_DATA_PORT);

        if (code == SCANCODE_LSHIFT || code == SCANCODE_RSHIFT) {
            shift_held = TRUE;
            continue;
        }
        if (code == SCANCODE_LSHIFT_REL || code == SCANCODE_RSHIFT_REL) {
            shift_held = FALSE;
            continue;
        }

        /* bit 0x80 = relâchement de touche : on ignore, on ne traite
         * que l'appui pour rester simple. */
        if (code & 0x80) continue;
        if (code >= 128) continue;

        char c = shift_held ? scancode_to_ascii_shift[code] : scancode_to_ascii[code];
        if (c != 0) return c;
    }
}

/* Lit une ligne complète (jusqu'à Entrée), avec gestion basique du backspace,
 * échos les caractères à l'écran, et tronque à la taille du buffer. */
void keyboard_read_line(char* buffer, int max_len) {
    int len = 0;
    while (1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putchar('\n');
            buffer[len] = '\0';
            return;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                vga_putchar('\b');
            }
            continue;
        }
        if (len < max_len - 1) {
            buffer[len++] = c;
            vga_putchar(c);
        }
    }
}
