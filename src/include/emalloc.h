#ifndef TPE_PROTOS_EMALLOC_H
#define TPE_PROTOS_EMALLOC_H

#include "logger.h"

// Malloc tradicional con un log en caso de error.
void * emalloc (size_t length);

#endif //TPE_PROTOS_EMALLOC_H
