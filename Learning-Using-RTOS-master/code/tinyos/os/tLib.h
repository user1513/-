/**
 * @brief tinyOS所用的通用数据结构实现
 * @author 01课堂 李述铜 http://01ketang.cc
 * @date 2017-06-01
 * @version 1.0
 * @copyright 版权所有，禁止用于商业用途
 */
#ifndef TLIB_H
#define TLIB_H

#include <stdint.h>

/**
 * @defgroup 位图结构 位图结构
 * @{
 */
// 位图类型
typedef struct {
    uint32_t bitmap;       /**< 该位图只支持最大32位，如果需要的可扩充至更多位 */
} tBitmap;

void tBitmapInit (tBitmap *bitmap);
uint32_t tBitmapPosCount (void);
void tBitmapSet (tBitmap *bitmap, uint32_t pos);
void tBitmapClear (tBitmap *bitmap, uint32_t pos);
uint32_t tBitmapGetFirstSet (tBitmap *bitmap);

/** @} */ // 模块结尾

/**
 * @defgroup 双向链表 双向链表
 * @{
 */
// tinyOS链表的结点类型
typedef struct _tNode {
    struct _tNode *preNode;             /**< 前一结点 */
    struct _tNode *nextNode;            /**< 后一结点 */
} tNode;

void tNodeInit (tNode *node);

// tinyOS链表类型
typedef struct _tList {
    tNode headNode;
    uint32_t nodeCount;
} tList;

#define tNodeParent(node, parent, name) (parent *)((uint32_t)node - (uint32_t)&((parent *)0)->name)

void tListInit (tList *list);
uint32_t tListCount (tList *list);
tNode *tListFirst (tList *list);
tNode *tListLast (tList *list);
tNode *tListPre (tList *list, tNode *node);
tNode *tListNext (tList *list, tNode *node);
void tListRemoveAll (tList *list);
void tListAddFirst (tList *list, tNode *node);
void tListAddLast (tList *list, tNode *node);
tNode *tListRemoveFirst (tList *list);
void tListInsertAfter (tList *list, tNode *nodeAfter, tNode *nodeToInsert);
void tListRemove (tList *list, tNode *node);

/** @} */

/**
 * @defgroup 单向链表 单向链表
 * @{
 */
/**
 * 单向链表结点
 */
typedef struct _tSnode {
    struct _tSnode * next;                  /**<  后一结点 */
}tSnode;

void tSnodeInit (tSnode * snode);

/**
 * 单向链表
 */
typedef struct _tSlist {
    tSnode * firstNode;                     /**<  第一个结点 */
    tSnode * lastNode;                      /**<  最后一个结点 */
    uint32_t nodeCount;                     /**<  总的结点数量 */
}tSlist;

void tSlistInit (tSlist * slist);
uint32_t tSlistCount (tSlist * slist);
tSnode * tSlistFirst (tSlist * slist);
tSnode * tSlistLast (tSlist * slist);
void tSListAddFirst (tSlist *slist, tSnode * snode);
void tSListAddLast (tSlist *slist, tSnode * snode);
tSnode * tSListRemoveFirst (tSlist * slist);

/** @} */

#endif /* TLIB_H */
