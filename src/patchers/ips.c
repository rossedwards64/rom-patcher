#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"
#include "error.h"
#include "ips.h"

#ifdef DEBUG
#define PEEK(source) (*source->data)
#endif

#define READ(source) (*(source->data++))

#define READ_2(source)                \
    __extension__({                   \
        uint8_t byte1 = READ(source); \
        uint8_t byte2 = READ(source); \
        (byte1 << 8) | byte2;         \
    })

#define READ_3(source)                        \
    __extension__({                           \
        uint8_t byte1 = READ(source);         \
        uint8_t byte2 = READ(source);         \
        uint8_t byte3 = READ(source);         \
        (byte1 << 16) | (byte2 << 8) | byte3; \
    })

static char *get_file_name(const char *path)
{
    char *buf;
    while (*path++ != '\0');
    size_t name_length = 0;
    while (*--path != '/') { name_length++; }
    path++;
    buf = malloc(name_length);
    if (buf == nullptr) {
        printf("Failed to allocate %zu bytes for file name.\n", name_length);
        exit(ERROR_ALLOCATION_FAIL);
    }
    memcpy(buf, path, name_length);
    return buf;
}

static uint64_t get_file_size(FILE *file, const uint32_t limit)
{
    fseek(file, 0, SEEK_END);
    int64_t file_size = ftell(file);
    rewind(file);
    if (file_size > limit) {
        printf(
            "Patch file is %zu, larger than the maximum IPS patch file size "
            "%d.\n",
            file_size, MAX_PATCH_SIZE);
        exit(ERROR_FILE_TOO_LARGE);
    }
    return (uint64_t)file_size;
}

static source_t load_patch_data(const char *patch_path)
{
    FILE *patch_file = fopen(patch_path, "r");
    if (!patch_file) {
        printf("Failed to open file %s.\n", patch_path);
        exit(ERROR_FILE_OPEN);
    }

    uint64_t file_size = get_file_size(patch_file, MAX_PATCH_SIZE);
    uint8_t *data = malloc(file_size);
    if (data == nullptr) {
        printf("Failed to allocate %zu bytes for patch data.\n", file_size);
        exit(ERROR_ALLOCATION_FAIL);
    }

    uint64_t count = fread(data, sizeof(uint8_t), file_size, patch_file);
    if (count <= 0) {
        printf("Failed to read data from patch file.\n");
        exit(ERROR_FILE_DATA_READ);
    }
    fclose(patch_file);
    return (source_t){.data = data, .count = count, .idx = 0};
}

static bool check_string(const source_t *source, const char *expected)
{
    return memcmp(source->data, expected, strlen(expected)) == 0;
}

static void expect_string(source_t *source, const char *expected)
{
    bool found = check_string(source, expected);
    if (found) {
        source->data += strlen(expected);
    } else {
        printf("Expected string %s.\n", expected);
        exit(ERROR_STRING_NOT_FOUND);
    }
}

static void destroy_patch(ips_patch_t *patch)
{
    for (size_t i = 0; i < patch->size; i++) {
        ips_any_record_t record = patch->records[i];
        if (record.kind == RECORD) { SAFE_FREE(record.record.data); }
    }
    SAFE_FREE(patch->records);
    SAFE_FREE(patch->name);
}

static void check_if_at_end(source_t *source, const size_t bytes_ahead)
{
    if (source->idx + bytes_ahead >= source->count) {
        printf("Reached the end of the file without finding an EOF.\n");
        exit(ERROR_EOF_NOT_FOUND);
    }
}

static ips_any_record_t read_record(source_t *source)
{
    ips_any_record_t record;
    static constexpr size_t record_metadata_size = 5;
    check_if_at_end(source, record_metadata_size);
    uint32_t offset = READ_3(source);
    uint16_t size = READ_2(source);
    source->idx += record_metadata_size;

    if (size != 0) {
        uint8_t *record_data = malloc(size);
        if (record_data == nullptr) {
            printf("Failed to allocate %u bytes for record.\n", size);
            exit(ERROR_ALLOCATION_FAIL);
        }
        memcpy(record_data, source->data, size);
        source->data += size;
        source->idx += size;

        record = (ips_any_record_t){
            .kind = RECORD,
            .record = (ips_record_t){.data = record_data, .offset = offset}};
    } else {
        static constexpr size_t rle_metadata_size = 3;
        check_if_at_end(source, rle_metadata_size);
        uint16_t rle_size = READ_2(source);
        uint8_t rle_value = READ(source);
        source->idx += rle_metadata_size;

        ips_rle_record_t rle_record = (ips_rle_record_t){
            .offset = offset, .rle_size = rle_size, .rle_value = rle_value};
        record =
            (ips_any_record_t){.kind = RLE_RECORD, .rle_record = rle_record};
    }

    return record;
}

static void read_records(source_t *source, ips_patch_t *patch)
{
    for (size_t i = 0; !check_string(source, IPS_EOF); i++) {
        if (i >= patch->capacity) {
            size_t new_size = patch->capacity + INITIAL_RECORD_CAPACITY;
            patch->capacity = new_size;
            patch->records = realloc(
                patch->records, patch->capacity * sizeof(ips_any_record_t));
        }
        patch->records[i] = read_record(source);
        patch->size++;
    }
}

void read_patch(ips_patch_t *patch, source_t *source)
{
    /* save pointer returned from allocation in
       load_patch_data so it can be freed later */
    uint8_t *orig_data_ptr = source->data;

    expect_string(source, IPS_HEADER);
    read_records(source, patch);

    // restore the pointer and free
    source->data = orig_data_ptr;
    SAFE_FREE(source->data);
    orig_data_ptr = nullptr;
    printf("Read %zu records.\n", patch->size);
}

void apply_patch([[maybe_unused]] const char *rom_path)
{
    printf("Write IPS patch not yet implemented.\n");
}

void do_ips_patch(const char *patch_path, const char *rom_path)
{
    ips_patch_t patch = (ips_patch_t){
        .name = get_file_name(patch_path),
        .capacity = INITIAL_RECORD_CAPACITY,
        .size = 0,
        .records = malloc(INITIAL_RECORD_CAPACITY * sizeof(ips_any_record_t))};
    if (patch.records == nullptr) { exit(ERROR_ALLOCATION_FAIL); }
    source_t source = load_patch_data(patch_path);
    read_patch(&patch, &source);
    apply_patch(rom_path);
    destroy_patch(&patch);
}
