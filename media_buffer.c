#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "media_buffer.h"

int insert_buffer(struct BufferInfoList *buffer_list, char *buffer, int len)
{
    if (buffer == NULL || buffer_list == NULL) {
        return -1;
    }
    
    struct BufferInfo *buffer_info = (struct BufferInfo *)malloc(sizeof(struct BufferInfo));
    if (buffer_info == NULL) {
        return -1;
    }
    buffer_info->buffer = (char *)malloc(len);
    if (buffer_info->buffer == NULL) {
        return -1;
    }
    memcpy(buffer_info->buffer, buffer, len);
    buffer_info->len = len;
    buffer_info->next = NULL;

    pthread_mutex_lock(&(buffer_list->bufferMutex));
    if (buffer_list->currentbufferCount + 1 > buffer_list->maxBufferCount) { // buffer个数大于设定值，先删除最早的buffer
        struct BufferInfo *tmp_buffer = buffer_list->bufferList;
        buffer_list->bufferList = tmp_buffer->next;
        buffer_list->currentbufferCount--;
        free(tmp_buffer->buffer);
        free(tmp_buffer);
    }

    if (buffer_list->bufferList == NULL) {
        buffer_list->bufferList = buffer_info;
        buffer_list->lastbuffer = buffer_info;
    } else {
        buffer_list->lastbuffer->next = buffer_info;
        buffer_list->lastbuffer = buffer_info;
    }
    buffer_list->currentbufferCount++;
    // printf("media buffer count:%d\n",buffer_list->currentbufferCount);
    pthread_mutex_unlock(&(buffer_list->bufferMutex));
    return 0;
}

int get_buffer(struct BufferInfoList *buffer_list, struct BufferInfo **buffer)
{
    int ret = -1;
    if (buffer_list == NULL) {
        return -1;
    }

    pthread_mutex_lock(&(buffer_list->bufferMutex));
    if (buffer_list->bufferList != NULL) {
        *buffer = buffer_list->bufferList;
        buffer_list->bufferList = (*buffer)->next;
        buffer_list->currentbufferCount--;
        ret = 0;
    }
    pthread_mutex_unlock(&(buffer_list->bufferMutex));
    return ret;
}

int release_buffer(struct BufferInfo *buffer)
{
    if (buffer == NULL) {
        return 0;
    }
    free(buffer->buffer);
    buffer->len = 0;
    buffer->next = NULL;
    free(buffer);
    return 0;
}

int init_buffer_list(struct BufferInfoList *buffer_list, int max_frame)
{
    buffer_list->maxBufferCount = max_frame;
    buffer_list->currentbufferCount  = 0;
    buffer_list->bufferList = NULL;
    buffer_list->lastbuffer = NULL;
    pthread_mutex_init(&(buffer_list->bufferMutex), NULL);
    return 0;
}

int deinit_buffer_list(struct BufferInfoList *buffer_list)
{
    if (buffer_list == NULL) {
        return 0;
    }

    pthread_mutex_lock(&(buffer_list->bufferMutex));
    struct BufferInfo *tmp_buffer = buffer_list->bufferList;
    while(tmp_buffer != NULL) {
        buffer_list->bufferList = tmp_buffer->next;
        free(tmp_buffer->buffer);
        tmp_buffer->len = 0;
        tmp_buffer->next = NULL;
        free(tmp_buffer);
        tmp_buffer = buffer_list->bufferList;
    }
    pthread_mutex_unlock(&(buffer_list->bufferMutex));
    return 0;
}