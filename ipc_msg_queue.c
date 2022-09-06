#include "ipc_msg_queue.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <sys/time.h>

#include <pthread.h>

struct ipc_msg_queue {
    unsigned char *buf;
    int front;
    int rear;
    int max_msg;
    int msg_size;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

static int is_queue_full(struct ipc_msg_queue *q)
{
    return (q->front == (q->rear + 1) % (q->max_msg)) ? 1 : 0;
}

static int is_queue_empty(struct ipc_msg_queue *q)
{
    return (q->front == q->rear) ? 1 : 0;
}

int ipc_msg_queue_create(struct ipc_msg_queue **queue, const char *name, int msg_size, int max_msg)
{
    struct ipc_msg_queue *q = (struct ipc_msg_queue*)calloc(sizeof(struct ipc_msg_queue), 1);
    if (q == NULL) {
        return -1;
    }

    q->buf = (unsigned char*)malloc(msg_size*(max_msg + 1));
    if (q->buf == NULL) {
        free(q);
        return -1;
    }

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);

    q->front = q->rear = 0;
    q->max_msg = max_msg;
    q->msg_size = msg_size;

    *queue = q;
    return 0;
}

void ipc_msg_queue_destroy(struct ipc_msg_queue *queue)
{
    if (queue == NULL) {
        return ;
    }

    free(queue->buf);
    free(queue);
}

int ipc_msg_queue_send(struct ipc_msg_queue *queue, void *msg, unsigned int len, int timeout)
{
    if (queue == NULL) {
        return -1;
    }

    if (len != queue->msg_size) {
        return -1;
    }

    int error = 0;
    int empty = 0;

    pthread_mutex_lock(&queue->mutex);
    if (is_queue_empty(queue) == 1) {
        empty = 1;
    }
    while (is_queue_full(queue) == 1) {
        if (timeout == IPC_MSG_QUEUE_WAIT_FOREVER) {
            error = pthread_cond_wait(&queue->cond, &queue->mutex);
            if (error != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return error;
            }
        } else {
            struct timespec ts;
            struct timeval tv;
            gettimeofday(&tv, NULL);

            ts.tv_sec = tv.tv_sec + (timeout / 1000);
            ts.tv_nsec = tv.tv_usec * 1000 + (timeout % 1000) * 1000000;

            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            error = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
            if (error != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return error;
            }
        }
    }

    memcpy(queue->buf + queue->msg_size * queue->rear, msg, queue->msg_size);
    queue->rear = (queue->rear + 1) % (queue->max_msg);

    if (empty == 1) {
        pthread_cond_signal(&queue->cond);
    }
    pthread_mutex_unlock(&queue->mutex);

    return error;
}

int ipc_msg_queue_recv(struct ipc_msg_queue *queue, void *msg, int timeout)
{
    if (queue == NULL) {
        return -1;
    }

    int error = 0;

    pthread_mutex_lock(&queue->mutex);
    while (is_queue_empty(queue) == 1) {
        if (timeout == IPC_MSG_QUEUE_WAIT_FOREVER) {
            error = pthread_cond_wait(&queue->cond, &queue->mutex);
            if (error != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return error;
            }
        } else {
            struct timespec ts;
            struct timeval tv;
            gettimeofday(&tv, NULL);

            ts.tv_sec = tv.tv_sec + (timeout / 1000);
            ts.tv_nsec = tv.tv_usec * 1000 + (timeout % 1000) * 1000000;

            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            error = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
            if (error != 0) {
                pthread_mutex_unlock(&queue->mutex);
                return error;
            }
        }
    }

    memcpy(msg, queue->buf + queue->msg_size * queue->front, queue->msg_size);
    queue->front = (queue->front + 1) % (queue->max_msg);
    pthread_mutex_unlock(&queue->mutex);
    return error;
}

int ipc_msg_queue_query(struct ipc_msg_queue *queue, int *used, int *remained)
{
    if (queue == NULL) {
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);
    if (used != NULL) {
        *used = queue->front - queue->rear;
    }

    if (remained != NULL) {
        *remained = queue->max_msg - (queue->front - queue->rear);
    }

    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int ipc_msg_queue_flush(struct ipc_msg_queue *queue)
{
    if (queue == NULL) {
        return -1;
    }

    pthread_mutex_lock(&queue->mutex);
    queue->front = queue->rear = 0;
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

