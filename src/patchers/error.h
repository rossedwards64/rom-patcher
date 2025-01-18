#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERROR_FILE_OPEN,
    ERROR_FILE_TOO_LARGE,
    ERROR_FILE_DATA_READ,
    ERROR_ALLOCATION,
    ERROR_STRING_NOT_FOUND
} error_code_t;

#endif /* ERROR_H */
