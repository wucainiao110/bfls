/*
 KMP�ַ������ң�����㷨Ч�ʱȲ���BM��������BM
*/

#ifndef __BFLS_KMP_H
#define __BFLS_KMP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  
{
	int *next;
    int count;
	unsigned char sd[256]; // sunday�㷨
}kmp_t;

kmp_t* kmp_build_tbl(const unsigned char *pattern, unsigned int len);
int kmp_search(const unsigned char *dstbuffer, unsigned int dstlen, const unsigned char *pattern, unsigned int patlen, const kmp_t *kmp);


#ifdef __cplusplus
}
#endif


#endif

