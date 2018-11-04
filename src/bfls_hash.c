/*
  ��ϣ��ʵ�֡�
  ����ʹ����˫��������ά����ϣ�������elements������������
  1�����е�elements������
  2�������ݵ�elements������
  Ϊ����64λϵͳ�н�ʡ�ռ䣬����void*����int��ָ�򣬵�����С��2G��ʱ��
  ��û������ġ�
  �����ɢ�к������ǳ����˵ġ�
  
  1, �տ�ʼ���еĽڵ㶼�����ڿ����б���
  free_list ---->NODE1--->NODE2--->NODE3--->NODE4--->NODE5
  2, ��������ϣ�����Ԫ����NODE2��
  free_list ---->NODE1------------>NODE3--->NODE4--->NODE5
                          NODE2
  3, ���������һ��Ԫ�أ�NODE2��ͻ���������ͷժȡһ��
  free_list ---------------------->NODE3--->NODE4--->NODE5
                          NODE2--->NODE1
  4, ���������һ��Ԫ�أ���ϣֵ����NODE1����NODE1��λ��ȡ��һ������ڵ�
  free_list ------------------------------->NODE4--->NODE5
                NODE1     NODE2--->NODE3
  ����Ϊ�˱����ͻ��ʱ����Ҫ�Խڵ����ݽ��бȶԣ����滹������
  sub hash�ķ�����ֻ��hash��sub hash��һ����ʱ�����Ҫ��������
  �Ƚϣ�֮ǰ������4�����ַ������ԣ�hash��sub hash����ͬ��ֻ��
  һ��
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bfls_hash.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_DEBUG

/* ���ݹ�ϣֵ�����������ӹ�ϣֵ����������õ�λ�����ӹ�ϣ�ø�λ�����򷴹��� */
#define BFLS_HASH_INDEX(tbl, hash)   (((tbl)->shift==-1)?((hash)%(tbl)->elenum):(((hash)>>((tbl)->shift))%(tbl)->elenum))
#define BFLS_HASH_SUBHS(tbl, hash)   (((tbl)->shift==-1)?((hash)>>16):((hash)&0xffff))

/* table flags */
#define BFLS_HASH_TFLAG_DONOT_FREE      (1<<0)

/* data flags */
#define BFLS_HASH_DFLAG_MASTER      (1<<31) //����ӵ������ǡ���ͳ���ģ����Ǳ����õģ�
#define BFLS_HASH_DFLAG_SLAVE       (1<<30) //������Ǳ����õ�

/* element ���ݽṹ */
typedef struct
{
    int next;               // ����ָ�룬64λϵͳ�˷�4���ֽڣ���int������-1
    int prev;    
    unsigned int flags;     // ��16λ��flags����16λ��sub hash��Ҳ����һ����ϣֵ
    void *data;             // ����ָ��
#ifdef HASH_DEBUG
    unsigned int conflict;  // ����ӵĳ�ͻ��
#endif
}hash_data_t;

/* table���ݽṹ */
typedef struct
{
    bfls_hash_hashf   hash; // ɢ�к���
    bfls_hash_cmpf    cmp;  // �ȽϺ���
    unsigned int elenum;    // element number
    int          shift;     // ��ϣ����ȡ��λ���ߵ�λ������ø�λ����ͨ����λʵ�֣���λͨ��ȡ��ʵ�֡���λʱshift=-1
    unsigned int count;     // ��ǰelements��Ŀ
    unsigned int flags;     // flags
    int          next;      // free list
    hash_data_t* data;      // elements
#ifdef HASH_DEBUG
    unsigned int maxcfl;    // �����������ͻ��
    unsigned int maxcflidx; // ����ͻ����λ��
    unsigned int mastercnt; // ɢ����Ŀ
#endif
}hash_tbl_t;

/*
 ��free list�������һ�� 
 i >= 0��ʾ����ָ����
 i == -1��ʾ�����һ��
*/
static int hdlist_alloc(hash_tbl_t *tbl, int i)
{
    if (i < 0) {
        i = tbl->next; 
        #ifdef HASH_DEBUG
        if (i == -1) { // ��Ӧ�ó���
            printf("ERROR, i==-1\n");
            exit(-1);
        }
        #endif
    }

    if (tbl->data[i].next != -1) {
        tbl->data[tbl->data[i].next].prev = tbl->data[i].prev;
    }
    if (tbl->data[i].prev != -1) {
        tbl->data[tbl->data[i].prev].next = tbl->data[i].next;
    } else {
        tbl->next = tbl->data[i].next;
    }
    
    return i;
}

/*
 ��hash_data_t�Ż�free list����
*/
static void hdlist_free(hash_tbl_t *tbl, int i)
{
    tbl->data[i].next = tbl->next;
    tbl->data[i].prev = -1;
    tbl->next = i;
}

/* ��elements list������� */
static hash_data_t* hdlist_get(hash_tbl_t *tbl, unsigned int index, unsigned int subhs, void *key)
{
    hash_data_t *tbd = &tbl->data[index];
    while (1) {
        if (subhs==(tbd->flags&0xffff) && 0==tbl->cmp(tbd->data, key)) {
            break;
        } else if (tbd->next != -1) {
            tbd = &tbl->data[tbd->next];
        } else {
            tbd = NULL;
            break;
        }
    }
    
    return tbd;
}

/* ����������Ҫ��λ����Ŀ */
static unsigned int hash_power2(unsigned int size)
{
    // TODO >0x80000000 �����
    unsigned int k = 0;
    unsigned int shift = 0x80000000;
    while (size <= shift) {
        shift >>= 1;
        ++k;
    }
    return k;
}

/*
 @Author Brian Kernighan & Dennis Ritchie 
 */
unsigned int bfls_hash_char(void *data)
{ 
    const char *str = (char*)data;
    register unsigned int hash = 0; 
    register unsigned int ch;
    while (ch = (unsigned int)*str++)  
    {         
        hash = hash * 131 + ch;
    }  
    return hash;  
}

/* From linux */
unsigned int bfls_hash_32(unsigned int val)
{
	return val * 0x9e370001UL;
}

/* �����ͷ+elements�Ĵ�С��bytes */
unsigned int bfls_hash_size(unsigned int elenum)
{
    return sizeof(hash_tbl_t)+sizeof(hash_data_t)*elenum;
}

/*
˵��: ������ϣ��
���: addr        �ڴ��ַ������ǿձ�ʾ�����߸����ڴ�ķ��䣬���Ϊ�����ڲ�����
      elenum      Ԫ�صĸ���
      uselow      1��ʾʹ�ù�ϣֵ�ĵ�λ��������0��ʾʹ�ø�λ
      func        ɢ�к���
      cmmp        �ȽϺ���
*/
void* bfls_hash_create(void *addr, unsigned int elenum, int uselow, bfls_hash_hashf func, bfls_hash_cmpf cmp)
{
    hash_tbl_t *tbl;
    int shift;
    unsigned int size;
    int i;
    
    shift = (1==uselow)?-1:hash_power2(elenum);
    size = bfls_hash_size(elenum);

    if (elenum < 2) {
        return NULL;
    } else if (NULL == addr) {
        tbl = (hash_tbl_t*)calloc(1, size);
        if (NULL == tbl)
            return NULL;
    } else {
        memset(addr, 0, size);
        tbl = (hash_tbl_t*)addr;
        tbl->flags = BFLS_HASH_TFLAG_DONOT_FREE;
    }

    tbl->data = (hash_data_t*)(tbl+1);
    tbl->hash = func;
    tbl->cmp = cmp;
    tbl->elenum = elenum;
    tbl->shift = shift;
    //tbl->subhs = 0x10000;
    /*TODO ��ʱɢ�е�0-0xffff֮�䣬������һ��60K�ı��������subhs֮��
      ֻ��12���ַ���������ͻ��ÿ�鶼ֻ�������ַ���*/
    
    for (i=(int)elenum-1; i>=0; --i) {
        tbl->data[i].next = i+1;
        tbl->data[i].prev = i-1;
    }
    tbl->data[elenum-1].next = -1;
    
    return (void*)tbl;
}

/* ���ٹ�ϣ�� */
void bfls_hash_destroy(void *tbl)
{
    if (0 == (BFLS_HASH_TFLAG_DONOT_FREE & ((hash_tbl_t*)tbl)->flags))
        free(tbl);
}

/* ���ҹ�ϣ�� */
void *bfls_hash_get(void *table, void *key)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    hash_data_t *tbd;
    unsigned int hash = tbl->hash(key);
    unsigned int index = BFLS_HASH_INDEX(tbl, hash);//hash%tbl->size;
    unsigned int subhs = BFLS_HASH_SUBHS(tbl, hash);//hash%tbl->subhs;

    tbd = &tbl->data[index];
    if ((0 != (BFLS_HASH_DFLAG_MASTER&tbd->flags)) 
     && (NULL != (tbd=hdlist_get(tbl, index, subhs, key)))) {
         return tbd->data;
    } else {
        return NULL;
    }
}

// TODO BOOL?
int bfls_hash_add(void *table, void *data)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    hash_data_t *tbd;
    unsigned int hash = tbl->hash(data);
    unsigned int index = BFLS_HASH_INDEX(tbl, hash);
    unsigned int subhs = BFLS_HASH_SUBHS(tbl, hash);
    int i;

    tbd = &tbl->data[index];

    if (tbl->count >= tbl->elenum) {
        goto ERRLABEL; 
    } else if (0 != (BFLS_HASH_DFLAG_MASTER&tbd->flags)) { //��ͻ���ӿ���������һ��
        if (NULL == hdlist_get(tbl, index, subhs, data)) {
            i = hdlist_alloc(tbl, -1);
            tbl->data[i].next = tbd->next;
            tbl->data[i].prev = index;
            tbl->data[i].flags = BFLS_HASH_DFLAG_SLAVE+subhs;
            tbl->data[i].data = data;

            if (tbd->next != -1) {
                tbl->data[tbd->next].prev = i;
            }
            tbd->next = i;

            #ifdef HASH_DEBUG
            ++tbd->conflict;
            if (tbd->conflict > tbl->maxcfl) {
                tbl->maxcfl = tbd->conflict;
                tbl->maxcflidx = index;
            }
            #endif
        } else {
            goto ERRLABEL;
        }
    } else if (0 != (BFLS_HASH_DFLAG_SLAVE&tbd->flags)) { //������ռ��
        i = hdlist_alloc(tbl, -1);
        memcpy(&tbl->data[i], tbd, sizeof(hash_data_t));
        tbl->data[tbd->prev].next = i;
        if (-1 != tbd->next) {
            tbl->data[tbd->next].prev = i;
        }
        
        tbd->next = -1;
        tbd->prev = -1;
        tbd->data = data;
        tbd->flags = BFLS_HASH_DFLAG_MASTER+subhs;
        #ifdef HASH_DEBUG
        tbd->conflict = 0;
        ++tbl->mastercnt;
        #endif
    } else {
        i = hdlist_alloc(tbl, index);

        tbd->next = -1;
        tbd->prev = -1;
        tbd->data = data;
        tbd->flags = BFLS_HASH_DFLAG_MASTER+subhs;
        #ifdef HASH_DEBUG
        tbd->conflict = 0;
        ++tbl->mastercnt;
        #endif
    }
    
    ++tbl->count;

    return 0;
ERRLABEL:
    return -1;// TODO
}


void *bfls_hash_del(void *table, void *key)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    hash_data_t *tbd;
    unsigned int hash = tbl->hash(key);
    unsigned int index = BFLS_HASH_INDEX(tbl, hash);//hash%tbl->size;
    unsigned int subhs = BFLS_HASH_SUBHS(tbl, hash);//hash%tbl->subhs;
    unsigned int i;
    
    tbd = &tbl->data[index]; 
    if (0 != (BFLS_HASH_DFLAG_MASTER&tbd->flags)) {
        tbd = hdlist_get(tbl, index, subhs, key);
        if (NULL == tbd) {
            return NULL;
        } else if (0 != (BFLS_HASH_DFLAG_MASTER&tbd->flags)) { //�׽ڵ㣬��ô����ɾ�ڶ����ڵ�
            --tbl->count;
            i = tbd->next;
            if (-1 != i) {
                void *deldata = tbd->data;
                tbd->data = tbl->data[i].data;
                tbd->flags = BFLS_HASH_DFLAG_MASTER+(tbl->data[i].flags & 0xffff);
                tbd->next = tbl->data[i].next;
                if (tbl->data[i].next != -1) {
                    tbl->data[tbl->data[i].next].prev = index;
                }
                tbl->data[i].flags = 0;
                hdlist_free(tbl, i);
                return deldata;
            } else {
                tbd->flags = 0;
                hdlist_free(tbl, index);
                return tbd->data;
            }
        } else { // slave
            --tbl->count;
            i = tbl->data[tbd->prev].next; // my index
            if (-1 != tbd->next) {
                tbl->data[tbd->next].prev = tbd->prev;
            }
            tbl->data[tbd->prev].next = tbd->next;
            tbd->flags = 0;
            hdlist_free(tbl, i);
            return tbd->data;
        }
    } else {
        return NULL;
    }
}

// return count
unsigned int bfls_hash_iter(void *table, bfls_hash_iterf iter, void *arg)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    hash_data_t *tbd;
    unsigned int i;
    unsigned int cnt = 0;
    
    for (i=0; i<tbl->elenum; ++i) {
        tbd = &tbl->data[i];
        if (0 != ((BFLS_HASH_DFLAG_MASTER|BFLS_HASH_DFLAG_SLAVE)&tbd->flags)) {
            if (0 == iter(tbd->data, arg))
                ++cnt;
            else
                break;
        }
    }
    
    return cnt;
}

void bfls_hash_show_index(void *table, unsigned int index)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    hash_data_t *tbd;
    int i = (int)index;

    tbd = &tbl->data[i];
    if (0 != (BFLS_HASH_DFLAG_MASTER&tbd->flags)) {
        while (-1 != i) {
            tbd = &tbl->data[i];
            printf("Index:%-8d subhs:%04x data:%s\n", i, tbd->flags&0xffff, tbd->data);
            i = tbd->next;
        }
    } else {
        printf("Not Master\n");
    }
}

void bfls_hash_show_key(void *table, void *data)
{
    hash_tbl_t  *tbl = (hash_tbl_t*)table;
    unsigned int hash = tbl->hash(data);
    unsigned int index = BFLS_HASH_INDEX(tbl, hash);//hash%tbl->size;
    
    bfls_hash_show_index(table, index);
}

void bfls_hash_show(void *table)
{
    hash_tbl_t *tbl = (hash_tbl_t*)table;
    printf("Hash address:%#x\nSize:%d\nCount:%d\nShift:%d\n", (int)table, tbl->elenum, tbl->count, tbl->shift);
#ifdef HASH_DEBUG
    printf("Max Conflict:%d\nMax conflict index:%d\nMaster count:%d\n", tbl->maxcfl, tbl->maxcflidx, tbl->mastercnt);
#endif
}

#ifdef __cplusplus
}
#endif
