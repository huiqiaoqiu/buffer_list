#ifndef __MEDIA_BUFFER__
#define __MEDIA_BUFFER__

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

//pthread_mutex_t AudioBufferGetPopMutex = PTHREAD_MUTEX_INITIALIZER;
struct BufferInfo {
    char *buffer;
    int len;
    struct BufferInfo *next;
};

struct BufferInfoList {
    struct BufferInfo *bufferList;
    struct BufferInfo *lastbuffer;
    pthread_mutex_t bufferMutex;
    int currentbufferCount;
    int maxBufferCount;
};

int insert_buffer(struct BufferInfoList *buffer_list, char *buffer, int len);
int get_buffer(struct BufferInfoList *buffer_list, struct BufferInfo **buffer);
int release_buffer(struct BufferInfo *buffer);

int init_buffer_list(struct BufferInfoList *buffer_list, int max_frame);
int deinit_buffer_list(struct BufferInfoList *buffer_list); // 释放全部buffer

#ifdef __cplusplus
}
#endif

#endif