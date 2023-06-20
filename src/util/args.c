#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "logger.h"
#include "users.h"
#include "args.h"

struct pop3_args pop3_args;

static unsigned short
port(const char *s) {
    char *end     = 0;
    const long sl = strtol(s, &end, 10);

    if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
        fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
        exit(1);
    }
    return (unsigned short)sl;
}

static uint32_t
token(const char *s) {
    char *end     = 0;
    uint32_t sl = strtol(s, &end, 10);

    if (end == s|| '\0' != *end
        || ((SHRT_MIN == sl || SHRT_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > UINT32_MAX) {
        fprintf(stderr, "token should in in the range of 0-4294967295: %s\n", s);
        exit(1);
    }
    return (uint32_t)sl;
}

static void
version(void) {
    fprintf(stderr, "Pop3 version 0.0\n"
                    "ITBA Protocolos de Comunicación 2023/1 -- Grupo 5\n"
                    "AQUI VA LA LICENCIA\n");
}

static void
usage(const char *progname)
{
    fprintf(stderr,
            "Usage: %s [OPTION] ...\n"
            "\n"
            "   -h               Imprime la ayuda y termina.\n"
            "   -l <pop3 addr>   Dirección donde servirá  el proxy POP3.\n"
            "   -L <conf  addr>  Dirección donde servirá  el servicio de management.\n"
            "   -p <pop3 port>   Puerto entrante conexiones POP3.\n"
            "   -o <conf port>   Puerto entrante conexiones management\n"
            "   -u <user:pass>   Registra usuario y contraseña de un usuario válido.\n"
            "   -d <directory>   Directorio donde se almacenarán los mails.\n"
            "   -t <token>       Token de autenticación para el servicio de management.\n"
            "   -v               Imprime información sobre la versión y termina.\n"
            "\n",
            progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct pop3_args *args) {
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de user

    args->pop3_addr = "::";
    args->pop3_port = 1110;

    args->mng_addr = "::";
    args->mng_port = 9090;

    int c;

    while (true) {
        c = getopt(argc, argv, "hl::L::p::o::u:vd:t:");
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
                args->pop3_addr = optarg;
                break;
            case 'L':
                args->mng_addr = optarg;
                break;
            case 'p':
                args->pop3_port = port(optarg);
                break;
            case 'o':
                args->mng_port = port(optarg);
                break;
            case 'u':
                if (user_add_basic(optarg) != 0) {
                    log(INFO, "Error adding user: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'd':
                args->pop3_directory = optarg;
                break;
            case 't':
                args->manager_token = token(optarg);
                break;
            case 'v':
                version();
                exit(0);
            default:
                fprintf(stderr, "Unknown argument %d.\n", c);
                exit(1);

        }
    }

    if (args->pop3_directory == NULL) {
        fprintf(stderr, "Argument -d is required.\n");
        exit(1);
    }

    if (optind < argc - 1) {
        fprintf(stderr, "Argument not accepted: ");
        while (optind < argc - 1)
        {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}