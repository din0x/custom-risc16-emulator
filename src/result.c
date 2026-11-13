#include "result.h"


void result_init(struct result *r) {
    r->err = NULL;
}

void result_deinit(struct result *r) {
    if(r && r->err) {
        free(r->err);
        r->err = NULL;
    }
}
