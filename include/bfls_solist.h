/* ������������ */

#ifndef __BFLS_SOLIST_H
#define __BFLS_SOLIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*solcmp)(struct snode_s *node, struct snode_s *key);

typedef struct sonode_s
{
    struct sonode_s *next;
}sonode_t;


typedef struct
{
    sonode_t *head;
    sonode_t *tail;
    solcmp   cmp;
    int      count;
}solist_t;

/* ��ȡǰһ���ڵ� */
#define SOLIST_GET_PREV(list, key) \
({\
    sonode_t *__tmp = SLIST_FIRST(list);\
    sonode_t *__pre = SLIST_FIRST(list);\
    while (NULL != __tmp) {\
        if (0 == list->cmp(__tmp, (sonode_t*)key)) {\
            break;\
        }\
        __pre = __tmp;\
        __tmp = SLIST_NEXT(__tmp);\
    }\
    __pre;\
})


/* ����һ���ڵ㣬����ڵ�����SOLIST_ADD */
#define SOLIST_INSERT(list, prev, node) \
do { \
    if (NULL != prev) {\
        ((sonode_t*)(node))->next = ((sonode_t*)(prev))->next;\
        ((sonode_t*)(prev))->next = (sonode_t*)(node);\
    } else if (NULL != (list)->head) { \
        ((sonode_t*)(node))->next = (list)->head;\
        (list)->head = (sonode_t*)(node);\
    } else {\
        (list)->head = (sonode_t*)(node);\
    }\
    if (((sonode_t*)(prev)) == (list)->tail) (list)->tail = (sonode_t*)(node);\
    ++(list)->count;\
} while(0)

/* ɾ��һ���ڵ㣬ɾ���ڵ�����SOLIST_DEL */
#define SOLIST_REMOVE(list, prev, node) \
do {\
    if (NULL != prev) {\
        ((sonode_t*)(prev))->next = ((sonode_t*)(node))->next;\
    } else {\
        (list)->head = ((sonode_t*)(node))->next;\
    }\
    if (((sonode_t*)(node)) == (list)->tail) (list)->tail = (sonode_t*)(prev);\
    --(list)->count;\
} while(0)


/* ����һ������ */
#define SOLIST_DEF(name, cmp)   slist_t name = {NULL, NULL, 0}
#define SOLIST_FIRST(list)      ((list)->head)
#define SOLIST_LAST(list)       ((list)->tail)
#define SOLIST_NEXT(node)       (((sonode_t*)(node))->next)

/* ����key��ȡһ���ڵ� */
#define SOLIST_GET(list, key) \
({\
    sonode_t *__tmp = SLIST_FIRST(list);\
    while (NULL != __tmp) {\
        if (0 == list->cmp(__tmp, (sonode_t*)key)) {\
            break;\
        }\
        __tmp = SLIST_NEXT(__tmp);\
    }\
    __tmp;\
})

/* ָ��keyɾ��һ���ڵ� */
#define SOLIST_DEL(list, key) \
({\
    sonode_t *__tmp = SLIST_FIRST(list);\
    sonode_t *__pre = NULL;\
    while (NULL != __tmp) {\
        if (0 == list->cmp(__tmp, (sonode_t*)key)) {\
            SLIST_REMOVE(list, __pre, __tmp);\
            break;\
        }\
        __pre = __tmp;\
        __tmp = SLIST_NEXT(__tmp);\
    }\
    __tmp;\
})

/* ����һ���ڵ� */
#define SOLIST_ADD(list, node)  \
do {\
    sonode_t *__prev = SOLIST_GET_PREV(list, node);\
    SOLIST_INSERT(list, __prev, node);\
}while(0)


#ifdef __cplusplus
}
#endif

#endif

