#ifndef IPS_H
#define IPS_H

#include <stddef.h>
#include <stdint.h>

#define IPS_HEADER "PATCH"
#define IPS_FILE_EXT "ips"
#define IPS_EOF "EOF"
#define MAX_PATCH_SIZE 7340032
#define MAX_ROM_SIZE = 2147483648

typedef struct {
    uint8_t *data;
} source_t;

typedef struct {
    uint8_t *data;
    uint32_t offset;
} ips_record_t;

typedef struct {
    uint32_t offset;
    uint16_t rle_size;
    uint8_t rle_value;
} ips_rle_record_t;

typedef enum { RECORD, RLE_RECORD } ips_record_kind_t;

typedef struct {
    union {
        ips_record_t record;
        ips_rle_record_t rle_record;
    };
    ips_record_kind_t kind;
} ips_any_record_t;

typedef struct {
    char *name;
    uint64_t capacity;
    uint64_t size;
    ips_any_record_t *records;
} ips_patch_t;

typedef struct {
    ips_patch_t patch;
    uint8_t *data;
} ips_patcher_t;

void do_ips_patch(const char *patch_path, const char *rom_path);

#endif /* IPS_H */
