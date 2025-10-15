#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <coresys/vfs.h>
#include <coresys/types.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8
#define COM5 0x5F8
#define COM6 0x4F8
#define COM7 0x5E8
#define COM8 0x4E8

/* Standard baud rate divisors */
#define SERIAL_SET_BAUD_50 (0x0900)
#define SERIAL_SET_BAUD_300 (0x0180)
#define SERIAL_SET_BAUD_600 (0x00C0)
#define SERIAL_SET_BAUD_2400 (0x0030)
#define SERIAL_SET_BAUD_4800 (0x0018)
#define SERIAL_SET_BAUD_9600 (0x000C)
#define SERIAL_SET_BAUD_19200 (0x0006)
#define SERIAL_SET_BAUD_38400 (0x0003)
#define SERIAL_SET_BAUD_57600 (0x0002)
#define SERIAL_SET_BAUD_115200 (0x0001)

#define SERIAL_PARITY_ODD (2)
#define SERIAL_PARITY_EVEN (1)
#define SERIAL_NO_PARITY (0)

#define SERIAL_FLOW_CTL_NONE (1)
#define SERIAL_FLOW_CTL_DSR_DTR (2)
#define SERIAL_FLOW_CTL_RTS_CTS (3)

typedef struct {
  unsigned short serial_port;
  bool irq_en;
  unsigned short baud_rate_div;
  bool fifo_mode_en;
  unsigned char stop_bits;
  unsigned char parity;
  unsigned char flow_control;
  unsigned char rfu[3];
} serial_conf_t;

bool serial_init(serial_conf_t* serial_conf);

int serial_received(unsigned short serial_port);
char serial_read(unsigned short serial_port);
int is_transmit_empty(unsigned short serial_port);
void serial_write(unsigned short serial_port, char charout);
void serial_write_string(unsigned short serial_port, char* string);
void serial_write_buffer(unsigned short serial_port, unsigned char* buf,
                         unsigned int len);
void serial_read_line(unsigned short serial_port, char* string);
void serial_read_buffer(unsigned short serial_port, unsigned char* buf,
                        unsigned int len);

ssize_t serial_fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                        off_t offset);
ssize_t serial_fs_read(struct fs_node* node, const void* buf, size_t nbyte,
                       off_t offset);

#endif