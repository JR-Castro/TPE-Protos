#ifndef MACROS_H
#define MACROS_H

// Retorna la cantidad de elementos de un arreglo
#define N(x) (sizeof(x)/sizeof(x[0]))

// Inicializa un buffer con data, avanzando el puntero de write
#define FIXBUF(b, data) buffer_init(&(b), N(data), (data)); \
                        buffer_write_adv(&(b), N(data))

#endif
