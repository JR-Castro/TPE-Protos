
#ifndef TPE_PROTOS_METRICS_H
#define TPE_PROTOS_METRICS_H

#include <stddef.h>
#include <stdint.h>

struct metrics {
    size_t currentConnections;
    size_t totalConnections;
    size_t bytesSent;
};

void metrics_init();

void metricsNewConnection();

void metricsConnectionClosed();

void metricsBytesSent(size_t bytes);

void getMetrics(struct metrics *copy);

#endif //TPE_PROTOS_METRICS_H
