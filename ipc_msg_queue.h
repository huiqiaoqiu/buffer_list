#ifndef _IPC_MSG_QUEUE_H_
#define _IPC_MSG_QUEUE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define IPC_MSG_QUEUE_WAIT_FOREVER		(0xffffffff)
#define IPC_MSG_QUEUE_NO_WAIT			(0)

struct ipc_msg_queue;

int ipc_msg_queue_create(struct ipc_msg_queue **queue, const char *name, int msg_size, int max_msg);
void ipc_msg_queue_destroy(struct ipc_msg_queue *queue);
int ipc_msg_queue_send(struct ipc_msg_queue *queue, void *msg, unsigned int len, int timeout);
int ipc_msg_queue_recv(struct ipc_msg_queue *queue, void *msg, int timeout);
int ipc_msg_queue_query(struct ipc_msg_queue *queue, int *used, int *remained);
int ipc_msg_queue_flush(struct ipc_msg_queue *queue);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#endif