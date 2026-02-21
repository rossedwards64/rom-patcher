#ifndef ERROR_H
#define ERROR_H

typedef enum {
    ERROR_FILE_OPEN = 1,
    ERROR_FILE_TOO_LARGE,
    ERROR_FILE_DATA_READ,
    ERROR_ALLOCATION_FAIL,
    ERROR_STRING_NOT_FOUND,
    ERROR_EOF_NOT_FOUND
} error_code_t;

#endif /* ERROR_H */
