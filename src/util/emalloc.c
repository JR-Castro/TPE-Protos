#include "emalloc.h"

#include <stdlib.h>
#include <stdio.h>

void * emalloc (size_t length) {
    void * memory = malloc (length);
    if (!memory) {
        log(ERROR, "malloc couldn't allocate %lu bytes", (long)(length));
    }

    return memory;
}