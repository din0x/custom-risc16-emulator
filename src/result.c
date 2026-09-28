#include "result.h"


void result_init(Result *r) {
    r->err = NULL;
}

void result_deinit(Result *r) {
    if(r && r->err) {
        free(r->err);
        r->err = NULL;
    }
}
