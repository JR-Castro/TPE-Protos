#include "queue.h"
#include <string.h>
#include "emalloc.h"

static node * removeElementRec(queue *q, node *prev, node *current, void *element,
                               bool (*compareElements)(const void *, const void *));

queue * newQueue (size_t elementSize) {
    queue * q = (queue *) malloc(sizeof(queue));
    if (q == NULL) {
        return NULL;
    }
    q->elementSize = elementSize;
    q->size = 0;
    q->head = q->tail = NULL;

    return q;
}

queue * add (queue * q, void * element) {
    if (q == NULL) {
        return NULL;
    }

    node * n = (node *) emalloc(sizeof(node));
    n->element = emalloc(q->elementSize);
    memcpy(n->element, element, q->elementSize);
    n->next = NULL;

    if (q->head == NULL) {
        q->head = q->tail = n;
    } else {
        q->tail->next = n;
        q->tail = n;
    }
    q->size++;

    return q;
}

void * poll (queue * q) {
    if (isEmpty(q)) {
        return NULL;
    }

    node * n = q->head;
    q->head = q->head->next;
    q->size--;
    void * element = emalloc(q->elementSize);
    memcpy(element, n->element, q->elementSize);
    free(n->element);
    free(n);

    return element;
}

void * peek (queue * q) {
    if (isEmpty(q)) {
        return NULL;
    }

    return q->head->element;
}

void * peekTail (queue * q) {
    if (isEmpty(q)) {
        return NULL;
    }

    return q->tail->element;
}

queue * removeElement(queue *q, void *element, bool (*compareElements)(const void *, const void *)) {
    q->head = removeElementRec(q, NULL, q->head, element, compareElements);

    if (q->head == NULL) {
        q->tail = NULL;
    }

    q->size--;
    return q;
}

static node * removeElementRec(queue *q, node *prev, node *current, void *element,
                               bool (*compareElements)(const void *, const void *)) {
    if (current == NULL) {
        return NULL;
    }
    node *aux;
    if (compareElements(current->element, element)) {
        aux = prev;
        node *next = current->next;
        if (current == q->tail) {
            q->tail = aux;
        }
        if (current == q->head) {
            q->head = current->next;
        }
        free(current);
        return next;
    }
    aux = current;
    current->next = removeElementRec(q, aux, current->next, element, compareElements);
    return current;
}

bool isEmpty(queue * q) {
    return size(q) == 0;
}

size_t size (queue * q) {
    return q->size;
}

queue * clearQueue(queue * q) {
    if (q == NULL) {
        return NULL;
    }

    while (!isEmpty(q)) {
        poll(q);
    }

    return q;
}

void destroyQueue(queue ** q) {
    if (q == NULL || *q == NULL) {
        return;
    }

    clearQueue(*q);
    free(*q);
    *q = NULL;
}

queueIterator * newQueueIterator(const queue *q) {
    queueIterator *iter = (queueIterator *) emalloc(sizeof(queueIterator));
    iter->queue = q;
    iter->current = q->head;

    return iter;
}

bool hasNext(const queueIterator *iter) {
    return iter->current != NULL;
}

void* next(queueIterator *iter) {
    if (!hasNext(iter)) {
        return NULL; // No more elements
    }

    void *element = iter->current->element;
    iter->current = iter->current->next;

    return element;
}