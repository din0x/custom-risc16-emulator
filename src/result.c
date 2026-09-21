#include "result.h"


void result_deinit(Result *r) {
    if(r && r->err) {
        free(r->err);
        r->err = NULL;
    }
}
