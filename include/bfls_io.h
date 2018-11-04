/* bfls_io.h */

#include <stdarg.h>

#ifndef __BFLS_IO_H
#define __BFLS_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/* ͨ�ø�ʽ���ص�������ch�Ǵ�������ַ���parg�ǻص��������� */
typedef void (*bflsiocb)(char ch, void *parg);

/* %U���ݸ� bflsiousrcb �Ļص����� */
typedef struct
{
    bflsiocb    pfunc;      /* bflsiousrcb���øú�����ʽ������ַ� */
    void*       cbparg;     /* ��bflsiousrcb�õĻص����� */
    void*       userarg;    /* %U��Ҫ���ݸ�������������һ��dlibiousercb, һ�� userarg*/
    int         finish;     /* �����ɣ�1��ʾ��ɣ�0��ʾ��Ҫ���� */
}bflsiousr_t;

/*
  ����: ��ʽ��������ַ���
*/
typedef unsigned int (*bflsiousrcb)(bflsiousr_t *ctx);

unsigned int bfls_format(bflsiocb pcb, void *pcbarg, const char *fmtstr, va_list arg);

unsigned int bfls_strlen(const char *str);
unsigned int bfls_printf(const char *fmtstr, ...);
unsigned int bfls_sprintf(char *buffer, const char *fmtstr, ...);
unsigned int bfls_fprintf(FILE *fp, const char *fmtstr, ...);
unsigned int bfls_dprintf(int fd, const char *fmtstr, ...);
unsigned int bfls_snprintf(char *buffer, unsigned int max, const char *fmtstr, ...);
unsigned int bfls_vprintf(const char *fmtstr, va_list arglist);
unsigned int bfls_vsprintf(char *buffer, const char *fmtstr, va_list arglist);
unsigned int bfls_vfprintf(FILE *fp, const char *fmtstr, va_list arglist);
unsigned int bfls_vdprintf(int fd, const char *fmtstr, va_list arglist);
unsigned int bfls_vsnprintf(char *buffer, unsigned int max, const char *fmtstr, va_list arglist);

#ifdef __cplusplus
}
#endif

#endif


