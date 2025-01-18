#include "patchers/bps.h"
#include "patchers/ips.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { IPS, BPS, NONE } patcher_type_t;

bool file_extension_matches(const char *path, const char *expected)
{
    const char *temp = path;
    bool matches = false;
    while (*temp++ != '\0');
    while (*--temp != '.');
    temp++;
    int result = strncmp(temp, expected, strlen(expected));
    if (result == 0) matches = true;
    return matches;
}

patcher_type_t choose_patcher(const char *patch_path)
{
    patcher_type_t result;
    if (file_extension_matches(patch_path, IPS_FILE_EXT)) {
        printf("Using IPS patcher...\n");
        result = IPS;
    } else if (file_extension_matches(patch_path, BPS_FILE_EXT)) {
        printf("Using BPS patcher...\n");
        result = BPS;
    } else {
        printf("Could not choose a patcher from path %s.\n", patch_path);
        result = NONE;
    }
    return result;
}

int main(int argc, char **argv)
{
    char *patch_path = nullptr;
    char *rom_path = nullptr;
    int opt;
    while ((opt = getopt(argc, argv, "p:r:")) != -1) {
        switch (opt) {
            case 'p':
                printf("Patch file is %s.\n", optarg);
                patch_path = optarg;
                break;
            case 'r':
                printf("ROM file is %s.\n", optarg);
                rom_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-pr]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    switch (choose_patcher(patch_path)) {
        case IPS: do_ips_patch(patch_path, rom_path); break;
        case BPS: do_bps_patch(patch_path, rom_path); break;
        case NONE:
        default: return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
