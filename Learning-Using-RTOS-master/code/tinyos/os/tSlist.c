/**
 * @brief tinyOS所用的单向链表数据结构
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */

/**
 * @defgroup 单向链表 单向链表
 * @{
 */

#include "tLib.h"

/**
 * 初始化单链表结点
 * @param snode 单链表结点
 */
void tSnodeInit (tSnode * snode) {
    snode->next = snode;
}

/**
 * 初始化单链表
 * @param slist 单链表
 */
void tSlistInit (tSlist * slist) {
    slist->firstNode = (tSnode *)0;
    slist->lastNode = (tSnode *)0;
    slist->nodeCount = 0;
}

/**
 * 获取单链表结点数量
 * @param slist 查询的单链表
 * @return 结点数量
 */
uint32_t tSlistCount (tSlist * slist) {
    return slist->nodeCount;
}

/**
 * 获取单链表的第一个结点
 * @param slist 查询的单链表
 * @return 第一个结点，如果没有，返回0
 */
tSnode * tSlistFirst (tSlist * slist) {
    return slist->firstNode;
}

/**
 * 获取单链表的最后一个结点
 * @param slist 查询的单链表
 * @return 最后一个结点，如果没有，返回0
 */
tSnode * tSlistLast (tSlist * slist) {
    return slist->lastNode;
}

/**
 * 将结点添加到链表表头
 * @param slist 操作的链表
 * @param snode 待插入的结点
 */
void tSListAddFirst (tSlist *slist, tSnode * snode) {
    if (slist->nodeCount == 0) {
        slist->firstNode = snode;
        slist->lastNode = snode;
        slist->nodeCount = 1;
    } else {
        snode->next = slist->firstNode;
        slist->firstNode = snode;
        slist->nodeCount++;
    }
}

/**
 * 将结点插入到链表尾部
 * @param slist 操作的链表
 * @param snode 待插入的结点
 */
void tSListAddLast (tSlist *slist, tSnode * snode) {
    if (slist->nodeCount == 0) {
        slist->firstNode = snode;
        slist->lastNode = snode;
        slist->nodeCount = 1;
    } else {
        slist->lastNode->next = snode;
        snode->next = snode;
        slist->lastNode = snode;
        slist->nodeCount++;
    }
}

/**
 * 移除链表的首个结点
 * @param slist 操作的链表
 * @return 移除的结点，如果没有，返回0
 */
tSnode * tSListRemoveFirst (tSlist * slist) {
    switch (slist->nodeCount) {
        case 0:
            return (tSnode *)0;
        case 1: {
            tSnode * removeNode = slist->firstNode;
            removeNode->next = removeNode;

            slist->firstNode = (tSnode *)0;
            slist->lastNode = (tSnode *)0;
            slist->nodeCount = 0;
            return removeNode;
        }
        default: {
            tSnode * removeNode = slist->firstNode;

            slist->firstNode = removeNode->next;
            removeNode->next = removeNode;
            slist->nodeCount--;
            return removeNode;
        }

    }
}
/** @} */

