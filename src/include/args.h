#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>

struct pop3_args {
    char * pop3_addr;
    unsigned short pop3_port;

    char * origin_addr;
    unsigned short origin_port;

    char * mng_addr;
    unsigned short mng_port;
}pop3_args;

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc, char **argv, struct pop3_args *args);

#endif

