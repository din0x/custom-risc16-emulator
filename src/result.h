#ifndef RESULT_H
#define RESULT_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char *err;
} Result;

#define ERR(...) do {                               \
    Result __result;                                \
    size_t __len = snprintf(NULL, 0, __VA_ARGS__);  \
    __result.err = (char *)malloc(__len + 1);       \
    snprintf(__result.err, __len, __VA_ARGS__);     \
    return __result;                                \
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
