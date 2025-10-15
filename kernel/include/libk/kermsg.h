#ifndef _KER_MSG_H_
#define _KER_MSG_H_

#include <libk/stdio.h>

#define FATAL_ERROR_LABEL                  "FATAL ERROR! -  "
#define KERNEL_INFO_LABEL                  "[INFO] "

#define kermsg_info(MESSAGE)               (printf(KERNEL_INFO_LABEL MESSAGE))

#define kermsg_lab_start(MESSAGE)          (printf("[ ]  " MESSAGE))
#define kermsg_lab_end_ok()                (printf("\r[OK]\n"))
#define kermsg_labend_ko()                 (printf("\r[X] \n"))

#define kermsg_print_fatal_error(MESSAGE)  (printf(FATAL_ERROR_LABEL MESSAGE))

#endif