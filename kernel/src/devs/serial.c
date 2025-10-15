#include <IA32/init_defs.h>
#include <IA32/io.h>
#include <coresys/types.h>
#include <devs/serial.h>
#include <libk/queue.h>
#include <libk/stdint.h>
#include <libk/stdio.h>
#include <libk/string.h>

/* Serial Registers */
#define SERIAL_DATA_REG (0)
#define SERIAL_INT_EN_REG (1)
#define SERIAL_BAUD_HIGH_REG (1)
#define SERIAL_BAUD_LOW_REG (0)
#define SERIAL_INT_ID_FIFO_CTL_REG (2)
#define SERIAL_LINE_CTL_REG (3)
#define SERIAL_MODEM_CTL_REG (4)
#define SERIAL_LINE_STATUS_REG (5)
#define SERIAL_MODEM_STATUS_REG (6)
#define SERIAL_SCRATCH_REG (7)

/* FIFO register sets */
#define SERIAL_FIFO_DISABLE (0x00)
#define SERIAL_FIFO_ENABLE (0x01)

/* Line Control Register */
#define SERIAL_LCR_DLAB_BIT (0x80)
#define SERIAL_LCR_SETBREAK_EN_BIT (0x40)
#define SERIAL_LCR_NO_PARITY (0x00)
#define SERIAL_LCR_ODD_PARITY (0x08)
#define SERIAL_LCR_EVEN_PARITY (0x18)
#define SERIAL_LCR_MARK_PARITY (0x28)
#define SERIAL_LCR_SPACE_PARITY (0x38)
#define SERIAL_LCR_ONE_STOP_BIT (0x00)
#define SERIAL_LCR_TWO_STOP_BITS (0x04)
#define SERIAL_LCR_WORD5_BITS (0x00)
#define SERIAL_LCR_WORD6_BITS (0x01)
#define SERIAL_LCR_WORD7_BITS (0x02)
#define SERIAL_LCR_WORD8_BITS (0x03)

/* Modem Control Register */
#define SERIAL_MODEM_DTR_BIT (0x01)
#define SERIAL_MODEM_RTS_BIT (0x02)
#define SERIAL_MODEM_AUX_OUT_1 (0x04)
#define SERIAL_MODEM_AUX_OUT_2 (0x08)
#define SERIAL_MODEM_LOOPBACK_MODE (0x10)
#define SERIAL_MODEM_AUTOFLOW_EN (0x20)

/* Line Status Register */
#define SERIAL_LSR_DR (0x01)  // Data Ready
#define SERIAL_LSR_OVERRUN_ERR (0x02)
#define SERIAL_LSR_PARITY_ERR (0x04)
#define SERIAL_LSR_FRAMING_ERR (0x08)
#define SERIAL_LSR_BREAK_I (0x10)
#define SERIAL_LSR_THRE (0x20)  // Transmitter holding register empty
#define SERIAL_LSR_TEMT (0x40)  // Transmitter empty
#define SERIAL_IMPENDING_ERR (0x80)

/* Interrupt Pending */
#define SERIAL_IIR_PENDING_BIT (0x01)

bool serial_init(serial_conf_t* serial_conf) {
  unsigned char out_val = 0;

  /* Baud Rate */
  // Enable DLAB
  outb(SERIAL_LCR_DLAB_BIT, serial_conf->serial_port + SERIAL_LINE_CTL_REG);
  // Set divisor low byte
  outb((unsigned char)(serial_conf->baud_rate_div & 0x0F),
       serial_conf->serial_port + SERIAL_BAUD_LOW_REG);
  // Set divisor high byte
  outb((unsigned char)(serial_conf->baud_rate_div >> 8),
       serial_conf->serial_port + SERIAL_BAUD_HIGH_REG);
  // Disable DLAB
  outb(0, serial_conf->serial_port + SERIAL_LINE_CTL_REG);

  /* Set Word len, parity and stop bits */
  if (serial_conf->parity == SERIAL_PARITY_EVEN) {
    out_val = SERIAL_LCR_EVEN_PARITY;
  } else if (serial_conf->parity == SERIAL_PARITY_ODD) {
    out_val = SERIAL_LCR_ODD_PARITY;
  } else {
    out_val = SERIAL_LCR_NO_PARITY;
  }

  /* 8-bit word */
  out_val |= SERIAL_LCR_WORD8_BITS;

  if (serial_conf->stop_bits == 2) {
    out_val |= SERIAL_LCR_TWO_STOP_BITS;
  } else {
    out_val |= SERIAL_LCR_ONE_STOP_BIT;
  }

  outb(out_val, serial_conf->serial_port + SERIAL_LINE_CTL_REG);

  /* FIFO Mode */
  if (serial_conf->fifo_mode_en) {
    out_val = 0xC7;
  } else {
    out_val = 0x00;
  }

  outb(out_val, serial_conf->serial_port + SERIAL_INT_ID_FIFO_CTL_REG);

  /* Test Serial */
  out_val = SERIAL_MODEM_LOOPBACK_MODE | SERIAL_MODEM_RTS_BIT;
  outb(out_val,
       serial_conf->serial_port + SERIAL_MODEM_CTL_REG);   // Set loopback mode
  outb(0xAE, serial_conf->serial_port + SERIAL_DATA_REG);  // Test data send

  // Readback data from the serial
  if (inb(serial_conf->serial_port + SERIAL_DATA_REG) != 0xAE) {
    // Test OK
    return false;
  }

  /* Flow Control */
  if (serial_conf->flow_control == SERIAL_FLOW_CTL_RTS_CTS) {
    out_val = SERIAL_MODEM_RTS_BIT;
  } else if (serial_conf->flow_control == SERIAL_FLOW_CTL_DSR_DTR) {
    out_val = SERIAL_MODEM_DTR_BIT;
  } else {
    out_val = 0;
  }

  /* Interrupt */
  if (!serial_conf->irq_en) {
    outb(out_val, serial_conf->serial_port +
                      SERIAL_MODEM_CTL_REG);  // Set in normal mode
    outb(0x00, serial_conf->serial_port + SERIAL_INT_EN_REG);
  } else {
    outb(SERIAL_MODEM_AUX_OUT_2 | out_val,
         serial_conf->serial_port + SERIAL_MODEM_CTL_REG);

    unsigned char onmask = inb(0x21);
    onmask |= ~(0x10);
    outb(onmask, 0x21);

    outb(0x01, serial_conf->serial_port + SERIAL_INT_EN_REG);
    outb(0x80, serial_conf->serial_port + SERIAL_MODEM_CTL_REG);
  }

  /* Read to cleen up LSR register */
  inb(serial_conf->serial_port + SERIAL_DATA_REG);

  return true;
}

int serial_received(unsigned short serial_port) {
  return inb(serial_port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_DR;
}

char serial_read(unsigned short serial_port) {
  while (serial_received(serial_port) == 0)
    ;

  return inb(serial_port + SERIAL_DATA_REG);
}

int is_transmit_empty(unsigned short serial_port) {
  return inb(serial_port + SERIAL_LINE_STATUS_REG) & SERIAL_LSR_THRE;
}

void serial_write(unsigned short serial_port, char charout) {
  while (is_transmit_empty(serial_port) == 0)
    ;

  outb(charout, serial_port + SERIAL_DATA_REG);
}

void serial_write_string(unsigned short serial_port, char* string) {
  unsigned int count;
  unsigned int len = strlen(string);
  for (count = 0; count < len; count++) {
    serial_write(serial_port, string[count]);
  }
}

void serial_write_buffer(unsigned short serial_port, unsigned char* buf,
                         unsigned int len) {
  unsigned int count;
  for (count = 0; count < len; count++) {
    serial_write(serial_port, buf[count]);
  }
}

void serial_read_line(unsigned short serial_port, char* string) {
  char read_char = 1;
  unsigned int count = 0;
  while (read_char != '\r') {
    read_char = serial_read(serial_port);
    if (read_char != '\r') {
      string[count] = read_char;
      count++;
    }
  }

  string[count] = 0;
}

void serial_read_buffer(unsigned short serial_port, unsigned char* buf,
                        unsigned int len) {
  unsigned int count;
  for (count = 0; count < len; count++) {
    buf[count] = serial_read(serial_port);
  }
}

ssize_t serial_fs_write(struct fs_node* node, const void* buf, size_t nbyte,
                        off_t __is_unused offset) {
  ret_t ret = RET_T_NOK;
  if (node != NULL) {
    serial_write_buffer(node->impl_def, (unsigned char*)buf,
                        (unsigned int)nbyte);
    ret = RET_T_OK;
  }
  return ret;
}

ssize_t serial_fs_read(struct fs_node* node, const void* buf, size_t nbyte,
                       off_t __is_unused offset) {
  ret_t ret = RET_T_NOK;
  if (node != NULL) {
    char in = '\0';
    size_t i;
    for (i = 0; i < nbyte; i++) {
      in = serial_read(node->impl_def);
      ((char*)buf)[i] = in;

      /* echo received char */
      serial_write(node->impl_def, in);
    }
    ret = RET_T_OK;
  }
  return ret;
}

void Serial_IRQ(uint32_t code) {
  unsigned short serial = COM1;

  /* TODO Serial IRQ mapping */
  if ((code - HW_INT_START) == 4) {
    serial = COM1;
  } else if ((code - HW_INT_START) == 3) {
    serial = COM2;
  }

  printf("Serial IRQ\n");
}