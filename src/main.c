#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { IPS, BPS } patcher_type_t;

bool file_extension_matches(const char *path, const char *expected)
{
    bool matches = false;
    while (*path++ != '\0');
    while (*--path != '.');
    *path++;
    int result = strncmp(path, expected, strlen(expected));
    if (result == 0) matches = true;
    return matches;
}

patcher_type_t choose_patcher(const char *patch_path)
{
    patcher_type_t result;
    if (file_extension_matches(patch_path, "ips")) {
        puts("Patching IPS file.");
        result = IPS;
    } else if (file_extension_matches(patch_path, "bps")) {
        puts("Patching BPS file.");
        result = BPS;
    }
    return result;
}

int main(int argc, char **argv)
{
    char *patch_path;
    char *rom_path;

    int opt;
    while ((opt = getopt(argc, argv, "p:r:")) != -1) {
        switch (opt) {
            case 'p':
                printf("Patch file is %s\n", optarg);
                patch_path = optarg;
                break;
            case 'r':
                printf("ROM file is %s\n", optarg);
                rom_path = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-pr]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    patcher_type_t patcher_type = choose_patcher(patch_path);

    return EXIT_SUCCESS;
}
