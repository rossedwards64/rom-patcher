#include <CUnit/Basic.h>
#include <CUnit/CUError.h>
#include <CUnit/CUnit.h>
#include <CUnit/TestDB.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../../src/patchers/error.h"
#include "../../src/patchers/ips.h"

constexpr const char SUITE_NAME[] = "ips";
constexpr const char HEADER_STR[] = "PATCH";
constexpr const char INCORRECT_HEADER_STR[] = "QBUDI";
constexpr const char RECORD_STR[] = "\x0\x1\xf\x0\x1\x1";
constexpr const char RLE_RECORD_STR[] = "\x0\x0\x0\x0\x0\xf\xf\x1";
constexpr const char EOF_STR[] = "EOF";
constexpr const char INCORRECT_EOF_STR[] = "FPG";
constexpr static uint64_t PATCH_SIZE =
    sizeof(HEADER_STR) + (sizeof(RECORD_STR) * 3) + (sizeof(RLE_RECORD_STR) * 3)
    + sizeof(EOF_STR);

char patch_path[FILENAME_MAX] = {0};

uint8_t *make_patch_data(const char *header, const char *eof)
{
    char *buf = malloc(PATCH_SIZE);
    strncat(buf, header, sizeof(HEADER_STR));
    for (size_t i = 0; i < 3; i++) {
        strcat(buf, RECORD_STR);
        strcat(buf, RLE_RECORD_STR);
    }
    strncat(buf, eof, sizeof(EOF_STR));
    return (uint8_t *)buf;
}

void run(ips_patch_t *patch, source_t *source, error_code_t expected_exit_code)
{
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        fclose(stderr);
        read_patch(patch, source);
        exit(0);
    }
    int status;
    wait(&status);
    CU_ASSERT(WEXITSTATUS(status) == (int)expected_exit_code);
}

void test_read_patch()
{
    ips_patch_t patch = (ips_patch_t){
        .name = nullptr,
        .capacity = INITIAL_RECORD_CAPACITY,
        .size = 0,
        .records = malloc(INITIAL_RECORD_CAPACITY * sizeof(ips_any_record_t))};
    source_t source =
        ((source_t){.data = make_patch_data(HEADER_STR, EOF_STR)});
    run(&patch, &source, 0);
    CU_ASSERT(patch.size == 0);
}

void test_read_path_exit_on_incorrect_header()
{
    ips_patch_t patch = (ips_patch_t){
        .name = nullptr,
        .capacity = INITIAL_RECORD_CAPACITY,
        .size = 0,
        .records = malloc(INITIAL_RECORD_CAPACITY * sizeof(ips_any_record_t))};
    source_t source =
        ((source_t){.data = make_patch_data(INCORRECT_HEADER_STR, EOF_STR)});
    run(&patch, &source, ERROR_STRING_NOT_FOUND);
}

void test_fails_on_incorrect_eof()
{
    ips_patch_t patch = (ips_patch_t){
        .name = nullptr,
        .capacity = INITIAL_RECORD_CAPACITY,
        .size = 0,
        .records = malloc(INITIAL_RECORD_CAPACITY * sizeof(ips_any_record_t))};
    source_t source =
        ((source_t){.data = make_patch_data(HEADER_STR, INCORRECT_EOF_STR)});
    run(&patch, &source, ERROR_EOF_NOT_FOUND);
}

int main(void)
{
    CU_pSuite test_suite = nullptr;
    if (CUE_SUCCESS != CU_initialize_registry()) return CU_get_error();

    test_suite = CU_add_suite(SUITE_NAME, NULL, NULL);
    if (test_suite == nullptr) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if ((CU_add_test(test_suite, "test reads correct patch", test_read_patch)
         == nullptr)
        || (CU_add_test(test_suite, "test fails reading on incorrect header",
                        test_read_path_exit_on_incorrect_header)
            == nullptr)
        || (CU_add_test(test_suite, "test fails reading on incorrect eof",
                        test_fails_on_incorrect_eof)
            == nullptr)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
