#include <stdint.h>

/* ── Circular Buffer ───────────────────────────────────── */

#define BUFFER_SIZE 10

typedef struct {
    float    data[BUFFER_SIZE]; /* ring of sensor readings */
    uint16_t head;              /* next write slot         */
    uint16_t tail;              /* oldest valid slot       */
    uint16_t count;             /* number of valid entries */
} CircularBuffer;

void cb_init(CircularBuffer *cb) {
    cb->head  = 0;
    cb->tail  = 0;
    cb->count = 0;
}

void cb_write(CircularBuffer *cb, float value) {
    cb->data[cb->head] = value;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    if (cb->count < BUFFER_SIZE)
        cb->count++;
    else
        cb->tail = (cb->tail + 1) % BUFFER_SIZE; /* drop oldest */
}

float cb_read_latest(CircularBuffer *cb) {
    int latest = (cb->head - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    return cb->data[latest];
}

/* ── Priority Queue ────────────────────────────────────── */

#define QUEUE_MAX 20

typedef enum { LOW = 0, MEDIUM = 1, HIGH = 2, CRITICAL = 3 } Priority;

typedef struct {
    uint8_t  sensor_id;  /* which sensor triggered (0-19) */
    uint8_t  priority;   /* severity level                */
    float    value;      /* the offending reading         */
    uint32_t timestamp;  /* ms since boot                 */
} Event;

typedef struct {
    Event events[QUEUE_MAX];
    int   size;
} PriorityQueue;

void pq_init(PriorityQueue *pq) {
    pq->size = 0;
}

void pq_insert(PriorityQueue *pq, Event e) {
    if (pq->size >= QUEUE_MAX) return; /* queue full, drop event */
    int i = pq->size - 1;
    while (i >= 0 && pq->events[i].priority < e.priority) {
        pq->events[i + 1] = pq->events[i];
        i--;
    }
    pq->events[i + 1] = e;
    pq->size++;
}

Event pq_dequeue(PriorityQueue *pq) {
    return pq->events[--pq->size];
}
