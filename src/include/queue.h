#ifndef TPE_PROTOS_QUEUE_H
#define TPE_PROTOS_QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct _node {
    void * element;
    struct _node * next;
} node;

typedef struct _queue {
    size_t size;
    size_t elementSize;
    node * head;
    node * tail;
} queue;

typedef struct _queueIterator {
    const queue *queue;
    node *current;
} queueIterator;

/**
 * @param elementSize Size of elements to be inserted
 * @return queue on success, NULL on error
 */
queue * newQueue(size_t elementSize);

/**
 * @param q The queue
 * @param data Data to be inserted
 * @return queue on success, NULL on error
 */
queue * add(queue * q, void * element);

/**
 * @brief Retrieves and removes the head of this queue,
 *        or returns null if this queue is empty.
 * @param q The queue
 * @return ptr to element on success, NULL if queue is empty
 */
void * poll(queue * q);

/**
 * @brief Retrieves, but does not remove, the head of this queue.
 * @param q The queue
 * @return ptr to element on success, NULL if queue is empty
 */
void * peek (queue * q);

/**
 * @brief Retrieves, but does not remove, the tail of this queue.
 * @param q The queue
 * @return ptr to element on success, NULL if queue is empty
 */
void * peekTail (queue * q);

/**
 * @brief Remove the element from the queue
 * @param q The queue
 * @param element Element to be removed
 * @return queue without element on success, NULL on error
 */
queue * removeElement(queue * q, void * element, bool (*compareElements)(const void *, const void *));

/**
 * @brief Delete all the elements of the queue
 * @param q The queue
 * @return queue on success, NULL on error
 */
queue * clearQueue(queue * q);

/**
 * @brief Destroy the queue
 * @param q A to the queue
 */
void destroyQueue(queue ** q);

/**
 * @param q The queue
 * @return size_t Size of queue
 */
size_t size(queue * q);

/**
 * @brief Check if queue is empty
 * @param q The queue
 * @return true if size >= 1, false otherwise,
 */
bool isEmpty(queue * q);

/**
 * @brief Initialize the queue iterator for the 'q' queue
 * @param q The queue
 * @return ptr to copy on success, NULL on error
 */
queueIterator * newQueueIterator(const queue * q);

/**
 * @brief Check if there are more elements in the iterator queue
 * @param q The queue
 * @return true if iter->current != NULL, false otherwise,
 */
bool hasNext(const queueIterator *iter);

/**
 * @brief Get the next element from the iterator queue
 * @param q The queue
 * @return ptr to element on success, NULL if queue has no more elements
 */
void* next(queueIterator *iter);

#endif
