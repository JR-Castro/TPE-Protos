#ifndef __logger_h_
#define __logger_h_
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>

/* 
*  Macros y funciones simples para log de errores.
*  EL log se hace en forma simple
*  Alternativa: usar syslog para un log mas completo. Ver sección 13.4 del libro de  Stevens
*/

typedef enum {DEBUG=0, INFO, ERROR, FATAL} LOG_LEVEL;

extern LOG_LEVEL current_level;

#define ERRVAL -1

/**
*  Minimo nivel de log a registrar. Cualquier llamada a log con un nivel mayor a newLevel sera ignorada
**/
void setLogLevel(LOG_LEVEL newLevel);

char * levelDescription(LOG_LEVEL level);

// Debe ser una macro para poder obtener nombre y linea de archivo. 
#define log(level, fmt, ...)   {if(level >= current_level) { \
	time_t loginternal_time = time(NULL); \
    struct tm loginternal_tm = *localtime(&loginternal_time); \
    fprintf (stderr, "%04d-%02d-%02dT%02d:%02d:%02d %s\t", \
            loginternal_tm.tm_year + 1900, loginternal_tm.tm_mon+1, loginternal_tm.tm_mday, \
            loginternal_tm.tm_hour, loginternal_tm.tm_min, loginternal_tm.tm_sec,\
            levelDescription(level)); \
	fprintf(stderr, fmt, ##__VA_ARGS__); \
	fprintf(stderr,"\n"); }\
	if ( level==FATAL) exit(1);}
#endif
