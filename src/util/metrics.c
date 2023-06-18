#include <string.h>

#include "metrics.h"

static struct metrics metrics;

void metrics_init() {
    memset(&metrics, 0, sizeof(struct metrics));
}

void metricsNewConnection() {
    metrics.currentConnections++;
    metrics.totalConnections++;
}

void metricsConnectionClosed() {
    metrics.currentConnections--;
}

void metricsBytesSent(size_t bytes) {
    metrics.bytesSent += bytes;
}

void getMetrics(struct metrics *copy) {
    memcpy(copy, &metrics, sizeof(struct metrics));
}
