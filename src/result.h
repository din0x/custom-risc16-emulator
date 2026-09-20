#ifndef RESULT_H
#define RESULT_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *err;
} Result;

#define ERR(__r, ...) do {                          \
    size_t __len = snprintf(NULL, 0, __VA_ARGS__);  \
    (__r)->err = (char *)malloc(__len + 1);         \
    snprintf((__r)->err, __len + 1, __VA_ARGS__);   \
} while(0);

#define OK do {                                     \
    Result __result = { 0 };                        \
    return __result;                                \
} while(0);

#define TRY(r) do {                                 \
    Result __result = r;                            \
    if(__result.err) {                              \
        return __result;                            \
    }                                               \
} while(0);

#endif
