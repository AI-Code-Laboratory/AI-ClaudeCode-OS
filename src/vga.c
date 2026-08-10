/* vga.c — pilote d'affichage texte VGA (mode 80x25, buffer à 0xB8000).
 * Pas de dépendance, pas de pilote graphique : c'est ce qui rend
 * l'interface "quasi-ASCII" et l'ensemble ultra léger. */

#include "vga.h"

static uint16_t* const VGA_BUFFER = (uint16_t*) 0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

static int cursor_row = 0;
static int cursor_col = 0;
static uint8_t current_color = 0x0A; /* vert clair sur noir, esprit "terminal" */

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t) c | ((uint16_t) color << 8);
}

void vga_set_color(uint8_t color) {
    current_color = color;
}

uint8_t vga_get_color(void) {
    return current_color;
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[y * VGA_WIDTH + x] = vga_entry(' ', current_color);
        }
    }
    cursor_row = 0;
    cursor_col = 0;
}

static void vga_scroll_if_needed(void) {
    if (cursor_row < VGA_HEIGHT) return;

    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_BUFFER[(y - 1) * VGA_WIDTH + x] = VGA_BUFFER[y * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', current_color);
    }
    cursor_row = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\r') {
        cursor_col = 0;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            VGA_BUFFER[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(' ', current_color);
        }
    } else {
        VGA_BUFFER[cursor_row * VGA_WIDTH + cursor_col] = vga_entry(c, current_color);
        cursor_col++;
        if (cursor_col >= VGA_WIDTH) {
            cursor_col = 0;
            cursor_row++;
        }
    }
    vga_scroll_if_needed();
}

void vga_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i]);
    }
}

void vga_print_int(int value) {
    if (value == 0) { vga_putchar('0'); return; }
    if (value < 0) { vga_putchar('-'); value = -value; }

    char buf[12];
    int i = 0;
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    while (i > 0) {
        vga_putchar(buf[--i]);
    }
}
