#ifndef _DES_H_
#define _DES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
	
typedef unsigned char uchar;
#define DES_FAIL (-1)
#define DES_SUCC 0

int des(uchar *binput, uchar *boutput, uchar *bkey);
int undes(uchar *binput, uchar *boutput, uchar *bkey);
int des3(uchar *binput, uchar *boutput, uchar *bkey);
int undes3(uchar *binput, uchar *boutput, uchar *bkey);
int des3_24(uchar *binput, uchar *boutput, uchar *bkey);
int undes3_24(uchar *binput, uchar *boutput, uchar *bkey);

#ifdef __cplusplus
}
#endif

#endif