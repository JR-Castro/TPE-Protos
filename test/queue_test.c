#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "queue.h"

START_TEST (integration_test_queue) {
    queue * q = newQueue(sizeof(int));
    ck_assert_uint_eq(0, size(q));
    ck_assert_ptr_eq(NULL, peek(q));

    q = add(q, &(int){1});
    q = add(q, &(int){3});
    q = add(q, &(int){5});
    ck_assert_uint_eq(3, size(q));
    ck_assert_uint_eq(1, *(int *)(peek(q)));
    ck_assert_uint_eq(5, *(int *)(peekTail(q)));
    ck_assert_uint_eq(3, size(q));
    ck_assert_uint_eq(1, *(int *)poll(q));
    ck_assert_uint_eq(5, *(int *)(peekTail(q)));
    ck_assert_uint_eq(3, *(int *)poll(q));

    q = clearQueue(q);
    ck_assert_uint_eq(0, size(q));
    destroyQueue(&q);
} END_TEST

START_TEST (test_empty_queue) {
    queue * q = newQueue(sizeof(void *));

    bool empty = isEmpty(q);
    ck_assert_int_eq(true, empty);

    ck_assert_ptr_eq(NULL, peek(q));
    ck_assert_ptr_eq(NULL, poll(q));

    destroyQueue(&q);
}

bool compareElements(const void * a, const void * b) {
    return *(int *)a == *(int *)b;
}

START_TEST (test_iterate_remove_queue) {
    queue * q = newQueue(sizeof(int));

    int one   = 1;
    int two   = 2;
    int three = 3;
    int four  = 4;
    int five  = 5;
    q = add(q, &one);
    q = add(q, &two);
    q = add(q, &three);
    q = add(q, &four);
    q = add(q, &five);

    printf("Before removing elements...\n");
    queueIterator * it = newQueueIterator(q);
    while (hasNext(it)) {
        int element = *(int *)next(it);
        printf("%d\n", element);
    }

    q = removeElement(q, &one,  compareElements);
    q = removeElement(q, &four, compareElements);
    q = removeElement(q, &five, compareElements);

    printf("After removing elements...\n");
    it = newQueueIterator(q);
    while (hasNext(it)) {
        int element = *(int *)next(it);
        printf("%d\n", element);
    }

    ck_assert_uint_eq(2, *(int *)(peek(q)));
    ck_assert_uint_eq(3, *(int *)(peekTail(q)));
    ck_assert_uint_eq(2, size(q));

} END_TEST

Suite *
suite(void) {
    Suite *s;
    TCase *tc;

    s = suite_create("queue_utils");
    tc = tcase_create("queue_utils");

    tcase_add_test(tc, integration_test_queue);
    tcase_add_test(tc, test_empty_queue);
    tcase_add_test(tc, test_iterate_remove_queue);
    suite_add_tcase(s, tc);

    return s;
}

int
main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
