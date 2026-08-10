#ifndef WINDOS_VGA_H
#define WINDOS_VGA_H

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef int             bool_t;
#define TRUE 1
#define FALSE 0

/* Couleurs VGA standard (avant-plan, à combiner avec un fond << 4) */
#define VGA_BLACK        0x0
#define VGA_GREEN        0x2
#define VGA_LIGHT_GREEN  0xA
#define VGA_CYAN         0x3
#define VGA_LIGHT_CYAN   0xB
#define VGA_WHITE        0xF
#define VGA_YELLOW       0xE
#define VGA_RED          0x4
#define VGA_LIGHT_RED    0xC
#define VGA_MAGENTA      0x5

void vga_set_color(uint8_t color);
uint8_t vga_get_color(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_print_int(int value);

#endif
