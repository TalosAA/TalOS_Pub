#include <IA32/io.h>
#include <coresys/vfs.h>
#include <devs/vga.h>
#include <hal/platform.h>
#include <libk/stddef.h>
#include <libk/stdint.h>
#include <libk/string.h>

static const size_t VGA_WIDTH = VGA_COLS;
static const size_t VGA_HEIGHT = VGA_ROWS;
static uint16_t* VGA_MEMORY = (uint16_t*)(VGA_BASE_ADDRESS);

static size_t vga_row;
static size_t vga_column;
static uint8_t vga_color;
static uint16_t* vga_buffer;

void vga_initialize(void) {
  vga_row = 0;
  vga_column = 0;
  vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  vga_buffer = VGA_MEMORY;
  vga_flush();
}

void vga_set_base_address(uint16_t* addr) { vga_buffer = addr; }

void vga_setcolor(uint8_t color) { vga_color = color; }

void vga_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  vga_buffer[index] = vga_entry(c, color);
}

void vga_putchar(char c) {
  if (c == '\r') {
    /* Carriage Return */
    vga_column = 0;
  } else if (c == '\n') {
    /* New Line, equivalent to \r\n */
    vga_column = 0;
    if (++vga_row == VGA_HEIGHT) vga_row = 0;
  } else {
    unsigned char uc = c;
    vga_putentryat(uc, vga_color, vga_column, vga_row);
    if (++vga_column == VGA_WIDTH) {
      vga_column = 0;
      if (++vga_row == VGA_HEIGHT) vga_row = 0;
    }
  }
  return;
}

void vga_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++) vga_putchar(data[i]);
}

void vga_writestring(const char* data) { vga_write(data, strlen(data)); }

void vga_flush(void) {
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      vga_buffer[index] = vga_entry(' ', vga_color);
    }
  }
  vga_column = 0;
  vga_row = 0;
}

void vga_cursor_off(void) {
  outb(0x0A, 0x3D4);
  outb(0x20, 0x3D5);
}

void vga_cursor_on(size_t cursor_start, size_t cursor_end) {
  outb(0x0A, 0x3D4);
  outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);

  outb(0x0B, 0x3D4);
  outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}

void vga_update_cursor(size_t x, size_t y) {
  size_t pos = y * VGA_WIDTH + x;

  outb(0x0F, 0x3D4);
  outb((uint8_t)(pos & 0xFF), 0x3D5);
  outb(0x0E, 0x3D4);
  outb((uint8_t)((pos >> 8) & 0xFF), 0x3D5);
}

ssize_t vga_fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                        off_t __is_unused offset) {
  ret_t ret = RET_T_NOK;
  if (node != NULL) {
    vga_write((const char*)buf, nbyte);
    ret = RET_T_OK;
  }
  return ret;
}
