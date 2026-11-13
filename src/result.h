#ifndef RESULT_H
#define RESULT_H

#include <stdlib.h>
#include <stdio.h>


struct result {
    char *err;
};

void result_init(struct result *r);
void result_deinit(struct result *r);

#define ERR(__r, ...) do {                          \
    result_deinit(__r);                             \
    size_t __len = snprintf(NULL, 0, __VA_ARGS__);  \
    (__r)->err = (char *)malloc(__len + 1);         \
    snprintf((__r)->err, __len + 1, __VA_ARGS__);   \
} while(0)

#define OK do {                                     \
    struct result __result = { 0 };                 \
    return __result;                                \
} while(0)

#define TRY(r) do {                                 \
    struct result __result = r;                     \
    if(__result.err) {                              \
        return __result;                            \
    }                                               \
} while(0);

#endif
