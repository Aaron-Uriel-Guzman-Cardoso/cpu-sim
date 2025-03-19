#include <queue.h>
#include <stdint.h>

int32_t
main(void) {
    struct queue *q = queue_new();
    if (!q) {
        return 1;
    }
    for (int32_t i = 0; i < 128; i += 1) {
        queue_enqueue(q, CPU_NONE);
    }
    for (int32_t i = 0; i < 64; i += 1) {
        queue_enqueue(q, CPU_HALT);
        queue_enqueue(q, CPU_REGISTER_OVERFLOW);
    }
    for (int32_t i = 0; i < 64; i += 1) {
        if (queue_dequeue(q) != CPU_HALT) {
            return 1;
        }
        if (queue_dequeue(q) != CPU_REGISTER_OVERFLOW) {
            return 1;
        }
    }
    queue_free(q);
    return 0;
}