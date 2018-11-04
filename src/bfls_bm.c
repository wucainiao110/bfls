#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bfls_bm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* �����ִ�Сд��map��Сдӳ����˴�д */
static const unsigned char case_map[256] = {
0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  
16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  
32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  
48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  
64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  
80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  
96,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  
80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  123, 124, 125, 126, 127, 
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

bm_t *bm_build_tbl(const unsigned char *pattern, unsigned int len)
{
    int  i = (int)len - 1;
    int  j;
    int  k;
    int  last;
    bm_t *bm = (bm_t*)calloc(1, sizeof(bm_t)+len*sizeof(int));
    
    if (bm == NULL) {
        return NULL;
    }
    bm->gs = (int*)(bm+1);
    bm->pattern = pattern;
    bm->plen = len;

    for (i=0; i<256; ++i) {
        bm->bs[i] = len;
    }
    for (i=0; i<(int)len; ++i) {
        bm->bs[pattern[i]] = len-1-i;
    }

    /*
      ���������"ƥ��ǰ׺"������xxxxxabcd����a��ʱ��ƥ���ˣ�
      ��ô����Ҫ����xxxxx���Ƿ�����Ӵ�bcd,��bcdǰ����һ����ĸ��
      ���Ҳ�����a(�����a����ô�϶�ͬ����ƥ�䣬���ǰ��û)�ַ�
      ��Ҳ����ƥ�䡣�������ַ���
      xyzefghijrbcdmnopqabcd�в���
      rbcdmnopqabcd
      �Ӻ���ǰƥ���ʱ��ƥ�䵽a��ʱ��ʧ���ˣ����ʱ�����Ҫ��
      ����ǰ�������Ѿ�ƥ��ɹ����Ӵ���û�������������ַ�������
      ���������Ľ�����ת
      ���Ӵ��������
      Դ��|<------------pre--- --------->|<---c1--->|<-post->|  
      �ӣ�|<---p--->|<---c2--->|<---b--->|<---c1--->|
      ����ƥ�䵽���ұ�bc����ĵط�ʧ���ˣ���ô��һ��ƥ��Ӧ������ô��
      Դ��|<-------------pre------------>|<---c1--->|<-post->|
      �ӣ�                     |<---p--->|<---c2--->|<---b--->|<---c1--->|
                                         old                             new
      c2������ȫ����c1��Ҳ������c1��һ����׺�Ӽ�����ʱ��p����Ϊ0��Ȼ��b��������
      һ��ʧ���ƶ��ľ��붼����ͬ�ģ�Ҳ��c2��b+c1��һ���Ӵ�����
      �ƶ��ľ������Ӵ�����-(c2+p)
      zbaab
      55531
    */
    
    /* һ��ʼ�ٶ��Ӵ�����û��ǰ��׺ƥ�䣬��ôƫ���������ַ������� */
    for (i=0; i<(int)len-1; ++i) {
        bm->gs[i] = len;
    }
    bm->gs[i] = 1;

    i = (int)len-2;
    last = i; // ����һ�����޸�ƫ������
    while (i >= 0) {
        if (pattern[i] == pattern[len-1]) {
            for (j=(int)len-2, k=i-1; k>=0; --k, --j) {
                if (pattern[k] != pattern[j]) {
                    break;
                }
            }
            
            if (k < 0) {/* c1��c2���Ӵ� */
                j = (int)len-1-i;
                for (k=len-i-2; k>i; --k) {
                    if (bm->gs[k] == (int)len) {
                        bm->gs[k] = j;
                    }
                }
            }
            else if (bm->gs[j] == (int)len) {
                bm->gs[j] = (int)len-1-i;
            }
        }
        --i;
    }

    return bm;
}


int bm_search(const unsigned char *src, unsigned int slen, unsigned int pos, const bm_t *bm)
{
    const unsigned char *pattern = bm->pattern;
    unsigned int plen = bm->plen;
    int base = (int)pos;
    int pid = (int)plen;
    int sid = pos+pid;
    
    while (sid <= (int)slen) {
        if (bm->bs[src[sid-1]] > 0) {
            base += bm->bs[src[sid-1]];
            sid += bm->bs[src[sid-1]];
        }

        while (pattern[--pid] == src[--sid]) {
            if (pid == 0) {
                return base;
            }
        }

        base += bm->gs[pid];
        pid = (int)plen;
        sid = base+pid;
    }
    
    return -1;
}

/* ��Сд�����а汾(case insensitive) */
bm_t *bm_build_tbl_ci(const unsigned char *pattern, unsigned int len)
{
    int  i = (int)len - 1;
    int  j;
    int  k;
    int  last;
    unsigned char *tpat;
    bm_t *bm = (bm_t*)calloc(1, sizeof(bm_t)+len*(sizeof(int)+sizeof(unsigned char)));
    
    if (bm == NULL) {
        return NULL;
    }
    bm->gs = (int*)(bm+1);
    tpat = (unsigned char*)(bm->gs+len);
    bm->plen = len;
    bm->pattern = tpat;

    for (i=0; i<(int)len; ++i) {
        tpat[i] = case_map[pattern[i]];
    }

    for (i=0; i<256; ++i) {
        bm->bs[i] = len;
    }
    for (i=0; i<(int)len; ++i) {
        bm->bs[bm->pattern[i]] = len-1-i;
    }

    /* һ��ʼ�ٶ��Ӵ�����û��ǰ��׺ƥ�䣬��ôƫ���������ַ������� */
    for (i=0; i<(int)len-1; ++i) {
        bm->gs[i] = len;
    }
    bm->gs[i] = 1;

    i = (int)len-2;
    last = i; // ����һ�����޸�ƫ������
    while (i >= 0) {
        if (tpat[i] == tpat[len-1]) {
            for (j=(int)len-2, k=i-1; k>=0; --k, --j) {
                if (tpat[k] != tpat[j]) {
                    break;
                }
            }
            
            if (k < 0) {/* c1��c2���Ӵ� */
                j = (int)len-1-i;
                for (k=len-i-2; k>i; --k) {
                    if (bm->gs[k] == (int)len) {
                        bm->gs[k] = j;
                    }
                }
            }
            else if (bm->gs[j] == (int)len) {
                bm->gs[j] = (int)len-1-i;
            }
        }
        --i;
    }

    return bm;
}


int bm_search_ci(const unsigned char *src, unsigned int slen, unsigned int pos, const bm_t *bm)
{
    const unsigned char *pattern = bm->pattern;
    unsigned int plen = bm->plen;

    int base = (int)pos;
    int pid = (int)plen;
    int sid = pos+pid;

    while (sid <= (int)slen) {
        if (bm->bs[case_map[src[sid-1]]] > 0) {
            base += bm->bs[case_map[src[sid-1]]];
            sid += bm->bs[case_map[src[sid-1]]];
        }

        while (pattern[--pid] == case_map[src[--sid]]) {
            if (pid == 0) {
                return base;
            }
        }

        base += bm->gs[pid];
        pid = (int)plen;
        sid = base+pid;
    }
    
    return -1;
}


#ifdef __cplusplus
}
#endif
