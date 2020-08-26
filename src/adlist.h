/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct listNode {
    struct listNode *prev;
    struct listNode *next;
    void *value; //数据 可以存放任何类型数据
} listNode;

typedef struct listIter {
    listNode *next;
    int direction; //迭代器遍历的方向 向后还是向前
} listIter;

typedef struct list {
    listNode *head;
    listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len; //链表长度
} list;

/* Functions implemented as macros */
//一些方便操作和阅读代码的宏定义 比如获取节点的值、获取头尾节点、设置获取struct函数等
#define listLength(l) ((l)->len)
#define listFirst(l) ((l)->head)
#define listLast(l) ((l)->tail)
#define listPrevNode(n) ((n)->prev)
#define listNextNode(n) ((n)->next)
#define listNodeValue(n) ((n)->value)

#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

#define listGetDupMethod(l) ((l)->dup)
#define listGetFreeMethod(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
//双端链表创建、释放、清空
list *listCreate(void);
void listRelease(list *list); //先调用listEmpty 然后释放list空间
void listEmpty(list *list);

//加入到头部、尾部、中间
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
 //after为true 则插入到后面，否则插入到前面
list *listInsertNode(list *list, listNode *old_node, void *value, int after);

void listDelNode(list *list, listNode *node);  //删除指定节点


//-------------迭代器--------------------
listIter *listGetIterator(list *list, int direction);//创建链表迭代器，从头、尾开始都可
listNode *listNext(listIter *iter); //获取迭代的下一个节点
void listReleaseIterator(listIter *iter); //释放迭代器


list *listDup(list *orig);//复制整个链表 返回复制后的地址
listNode *listSearchKey(list *list, void *key); //根据key搜索相同值得节点 head开始
listNode *listIndex(list *list, long index); //获取指定位置的节点 0代表第一个节点 支持负索引 -1代表倒数第一个


void listRewind(list *list, listIter *li); //将li迭代器重置到head节点
void listRewindTail(list *list, listIter *li); //重置到tail节点


//链表反转
void listRotateTailToHead(list *list);  //tail->head 左转
void listRotateHeadToTail(list *list);  //head->tail 右转


void listJoin(list *l, list *o);//链表连接 o 链接到 l 链表上



/* Directions for iterators */
//迭代器方向的宏定义
#define AL_START_HEAD 0 
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
