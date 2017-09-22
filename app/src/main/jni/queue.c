//
// Created by Administrator on 2017/9/20.
//
#include "./header/queue.h"
/**
 * 队列，这里主要用于存放AVPacket的指针
 * 这里，使用生产者消费者的模式老使用队列，至需要2个队列的实例，分别用来存储音频的AVPacket和视频的AVPacket
 * * 2个队列：
 *          音频AVPacket Queue
 *          视频AVPacket Queue
 * 1.生产者：read_stream线程负责不断的读取视频文件中的AVPacket，分别放到两个队列中
 * 2.消费者：
 *          1.视频解码，从视频AVPacket Queue中获取元素，解码，绘制
 *          2.音频解码，从音频AVPacket Queue中获取元素，解码，播放
 * 3个线程
 *        read_stream， decode_data_video， decode_data_audio
 *
 */
struct _Queue{
    //队列长度
    int size;
    //数组用二级指针, void 任意类型的指针数组，实质类型是AVPacket
//    AVPacket **packets;
    void  **tab;

    //队列读写的索引位置，按照先后顺序先后进行
    int nextReadIndex;
    int nextWriteIndex;

    int *ready;
    int count;//记录总的长度
};


/**
 * 初始化队列
 */
Queue *queue_init(int size, queue_fill_func fill_func){
    Queue *queue = malloc(sizeof(Queue));
    queue->size = size;
    queue->nextReadIndex = 0;
    queue->nextWriteIndex = 0;
    queue->count = 0;
    queue->tab = malloc(sizeof(*queue->tab) * size);
    int i;
    //事先开辟好空间，方便结构体间接赋值
    for(i=0; i<size; i++){
        //不知道初始化多大的空间，需要调用者告诉
        queue->tab[i] = fill_func();
    }
    return queue;
}

/**
 * 销毁队列
 */
void queue_free(Queue* queue,queue_free_func free_func){
    int i;
    for(i=0; i<queue->size; i++){
        //销毁队列的元素，通过使用回调函数
        free_func((void*)queue->tab[i]);
    }
    free(queue->tab);
    free(queue);
}


/**
 * 获取下一个索引位置
 */
int queue_get_next(Queue *queue, int current){
    return (current + 1) % queue->size;
}

/**
 * 队列添加元素
 */
void *queue_push(Queue *queue, pthread_mutex_t *mutex, pthread_cond_t *cond){

    while(queue->count == queue->size){ // 满了
        LOG_I_DEBUG("%s", "互斥锁push等待");
        pthread_cond_wait(cond, mutex);
    }
    LOG_I_DEBUG("queue_push:%d, queue:%#x", queue->count, queue);
    //通知消费者，有新的产品可以消费了
    queue->count++;
    pthread_cond_signal(cond);
    int next = queue->nextWriteIndex;
    queue->nextWriteIndex = queue_get_next(queue,next);
    return  queue->tab[next];
}


/**
 * 队列获取元素
 */
void * queue_pop(Queue *queue, pthread_mutex_t *mutex, pthread_cond_t *cond){
    while(queue->count == 0){ // 没了
        LOG_I_DEBUG("%s", "互斥锁pop等待");
        pthread_cond_wait(cond, mutex);
    }
    LOG_I_DEBUG("queue_pop:%d, queue:%#x", queue->count, queue);
    //通知生产者，没有产品了，可以继续了
    queue->count--;
    pthread_cond_signal(cond);
    int next = queue->nextReadIndex;
    queue->nextReadIndex = queue_get_next(queue,next);
    return  queue->tab[next];
}



