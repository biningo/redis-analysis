/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
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

#include <stdint.h>

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

//字典的k-v结构
typedef struct dictEntry {
    void *key;
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    //用来解决key的hash冲突 采用拉链法来解决 next表示下一个dictEntry 
    struct dictEntry *next;  //使用链表的"头插法"让插入的时间复杂度为O(1)
} dictEntry;

//操作dict的一组指定类型的函数集合 可以自由指定
typedef struct dictType {
    uint64_t (*hashFunction)(const void *key); //计算hash值
    void *(*keyDup)(void *privdata, const void *key);//复制dict的key的函数
    void *(*valDup)(void *privdata, const void *obj);//复制dict的value的函数
    int (*keyCompare)(void *privdata, const void *key1, const void *key2); //对比key
    void (*keyDestructor)(void *privdata, void *key); //销毁key
    void (*valDestructor)(void *privdata, void *obj); //销魂value
} dictType;


/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
//哈希表结构 hash table
typedef struct dictht {
    dictEntry **table; //hash table哈希表，dictEntry的数组，根据key的hash值来指定存储的kv索引  数组每个元素就是一个kv(dictEntry)
    unsigned long size; //hash table 大小
    unsigned long sizemask; //size-1  size+sizemask一起决定key的hash值
    unsigned long used; //哈希表已有节点数量 table数组有几个位置已经使用了
} dictht;

//字典结构体 里面包含了两个hash table 用来rehash
typedef struct dict {
    dictType *type; //特定类型的一组操作函数指针 用来操作dict
    void *privdata; 

    //两个hash table  用来rehash ht[1]只在rehash的时候使用
    dictht ht[2];  

    //记录rehash进度 表示完成rehash的key的数目  -1表示没有在rehash
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    unsigned long iterators; /* number of iterators currently running */
} dict;

/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct dictIterator {
    dict *d;
    long index;
    int table, safe;
    dictEntry *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
} dictIterator;

typedef void (dictScanFunction)(void *privdata, const dictEntry *de);
typedef void (dictScanBucketFunction)(void *privdata, dictEntry **bucketref);

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
//释放节点
#define dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)
//设置节点的值val
#define dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)
//设置string、int、float类型的值
#define dictSetSignedIntegerVal(entry, _val_) \
    do { (entry)->v.s64 = _val_; } while(0)

#define dictSetUnsignedIntegerVal(entry, _val_) \
    do { (entry)->v.u64 = _val_; } while(0)

#define dictSetDoubleVal(entry, _val_) \
    do { (entry)->v.d = _val_; } while(0)

//释放节点key的空间
#define dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

//设置节点的key
#define dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)

//比较两个key
#define dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dictHashKey(d, key) (d)->type->hashFunction(key)
#define dictGetKey(he) ((he)->key)
#define dictGetVal(he) ((he)->v.val)
#define dictGetSignedIntegerVal(he) ((he)->v.s64)
#define dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define dictGetDoubleVal(he) ((he)->v.d)
#define dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dictIsRehashing(d) ((d)->rehashidx != -1)

/* API */
//创建dict  默认HashTable设置为4
dict *dictCreate(dictType *type, void *privDataPtr);
//调整HashTable大小
int dictExpand(dict *d, unsigned long size);


//添加kv到字典
int dictAdd(dict *d, void *key, void *val);

//根据key判断是否在HashTable中，在则返回NULL并且设置existing为冲突节点，否则返回插入后的dictEntry 
//这里第3个字段exisiing带出来的是如果发生hash碰撞时的节点，方便插入到后面的链表中
dictEntry *dictAddRaw(dict *d, void *key, dictEntry **existing);

//调用dictAddRaw查找key，如果不存在key则添加到dict中，并且返回插入后的节点
//否则插入失败返回发生key对应的原来节点
dictEntry *dictAddOrFind(dict *d, void *key);

//添加kv 如果key已经存在 则用新值替换旧的值
int dictReplace(dict *d, void *key, void *val);


//删除指定的kv 并且释放空间
int dictDelete(dict *d, const void *key);
//删除指定的kv 但是返回dictEntry节点 不释放空间
dictEntry *dictUnlink(dict *ht, const void *key);
//释放 dictEntry *he的空间
void dictFreeUnlinkedEntry(dict *d, dictEntry *he);
//释放dict以及所有的kv
void dictRelease(dict *d);


//查找key代表的 dictEntry节点 不存在返回NULL 
dictEntry * dictFind(dict *d, const void *key);
//根据key获取value
void *dictFetchValue(dict *d, const void *key);

//将dict中HashTable的大小调整为最小
//但是 USED/BUCKETS 的比率要<= 1 调用dictExpand 实现
int dictResize(dict *d);


//dict迭代器
dictIterator *dictGetIterator(dict *d);
dictIterator *dictGetSafeIterator(dict *d);
dictEntry *dictNext(dictIterator *iter);
void dictReleaseIterator(dictIterator *iter);


//随机返回一个键值对
dictEntry *dictGetRandomKey(dict *d);
//在dictGetRandomKey 基础上做到更好的分散和随机性 减少重复
dictEntry *dictGetFairRandomKey(dict *d);
//返回指定count数目的kv 存储在des中
unsigned int dictGetSomeKeys(dict *d, dictEntry **des, unsigned int count);

void dictGetStats(char *buf, size_t bufsize, dict *d);

//生成hash值
uint64_t dictGenHashFunction(const void *key, int len);
uint64_t dictGenCaseHashFunction(const unsigned char *buf, int len);

//清空dict
void dictEmpty(dict *d, void(callback)(void*));

//设置dict 能否resize  就是设置dict_can_resize 标识位  1表示能 0表示不能
void dictEnableResize(void);
void dictDisableResize(void);

//开始rehash 传入n表示rehash的kv次数 
int dictRehash(dict *d, int n);
int dictRehashMilliseconds(dict *d, int ms);

//设置HashSeed 和获取 HashSeed
void dictSetHashFunctionSeed(uint8_t *seed);
uint8_t *dictGetHashFunctionSeed(void);


unsigned long dictScan(dict *d, unsigned long v, dictScanFunction *fn, dictScanBucketFunction *bucketfn, void *privdata);


//调用dictHashKey 来计算key的hash值 (仅仅是调用函数计算)
uint64_t dictGetHash(dict *d, const void *key);


dictEntry **dictFindEntryRefByPtrAndHash(dict *d, const void *oldptr, uint64_t hash);

/* Hash table types */
extern dictType dictTypeHeapStringCopyKey;
extern dictType dictTypeHeapStrings;
extern dictType dictTypeHeapStringCopyKeyValue;

#endif /* __DICT_H */
