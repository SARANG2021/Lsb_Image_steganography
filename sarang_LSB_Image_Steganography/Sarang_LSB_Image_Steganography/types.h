#ifndef TYPES_H
#define TYPES_H
#include <unistd.h>
/* User defined types */
typedef unsigned int uint;
typedef unsigned char uchar;

/* Status will be used in fn. return type */
typedef enum
{
    e_success,
    e_failure,
    d_success,
    d_failure
} Status;

typedef enum
{
    e_encode,
    e_decode,
    e_unsupported 
} OperationType;

#define print_sleep(fmt, ...) \
    do \
    { \
        usleep(500000); \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)

#endif
