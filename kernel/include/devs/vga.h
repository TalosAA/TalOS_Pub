#ifndef ARCH_IA32_VGA_H
#define ARCH_IA32_VGA_H

#include <libk/stddef.h>
#include <coresys/vfs.h>

#define VGA_BASE_ADDRESS     ((uint16_t*) 0xB8000)

enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

#define VGA_COLS   (80)
#define VGA_ROWS   (25)

void vga_initialize(void);
void vga_set_base_address(uint16_t* addr);
void vga_flush(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_writestring(const char* data);
void vga_cursor_on(size_t cursor_start, size_t cursor_end);
void vga_cursor_off(void);
void vga_update_cursor(size_t x, size_t y);

ssize_t vga_fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                        off_t offset);

/* Inline Functions */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t) uc | (uint16_t) color << 8;
}

#endif
