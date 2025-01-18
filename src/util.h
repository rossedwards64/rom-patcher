#ifndef UTIL_H
#define UTIL_H

#define SAFE_FREE(x)          \
    do {                      \
        if ((x) != nullptr) { \
            free(x);          \
            x = nullptr;      \
        }                     \
    } while (0)

#endif /* UTIL_H */
