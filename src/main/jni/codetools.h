/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	codetool.h
 * @brief	常用数据类型转换，该类从windows移植过来，做少量修改
 * @version	1.0
 * @author: ym
 * @date	2021/05/12
 ******************************************************************************/

#ifndef CODETOOLS_H_
#define CODETOOLS_H_


#include "pubdefine.h"
#include "rsa.h"

#include <unistd.h>


typedef unsigned char uchar;
typedef unsigned int  uint32;
typedef unsigned long word32;

#define TLVERR_BASE           -1100
#define NO_OBJSPACE     (TLVERR_BASE - 1)
#define DETAG_LENERR    (TLVERR_BASE - 2)
#define DELEN_LENERR    (TLVERR_BASE - 3)
#define ILL_TAGSTART    (TLVERR_BASE - 4)
#define NOT_SINGLEOBJ   (TLVERR_BASE - 5)
#define ILL_PARAMETER   (TLVERR_BASE - 6)

#define is_constructed(byte)      ((byte) & 0x20)
#define is_primitive(byte)        !is_constructed(byte)
#define has_subsequent(byte)      (((byte) & 0x1F) == 0x1F)
#define another_byte_follow(byte) ((byte) & 0x80)
#define lenlen_exceed_one(byte)   ((byte) & 0x80)

typedef struct 
{
	uint32 tagname;
	int parent;
	int childnum;
	int valuelen;
	uchar * pvalue;
}tlvobject;

#define SINGLE_TLVOBJ        1
#define STRING_TLVOBJ        0

#define SEARCH_ALL_DESC           1
#define SEARCH_ONLY_SON           0

#define ByteToBcd(n) ((((n) / 10) << 4) | ((n) % 10))
#define BcdToByte(n) (((n) >> 4) * 10 + ((n) & 0x0f))

#ifndef _sha1_uint8
#define _sha1_uint8  unsigned char
#endif

#ifndef _sha1_uint32
#define _sha1_uint32 unsigned long int
#endif

typedef struct 
{
	_sha1_uint32 total[2];
	_sha1_uint32 state[5];
	_sha1_uint8 buffer[64];
}
sha1_context;

class CCodeTools  
{
public:


	//static unsigned int ComputCrc32(unsigned int uCrc,unsigned char *Buf,int nLen);	
	//static unsigned int crc32_org(unsigned int crc, unsigned char *buf, size_t len);	
	//static int recur_decode(int parent,tlvobject * pobjects, int objspace);
	

	CCodeTools();
	virtual ~CCodeTools();
	static int ASC2BCD(unsigned char *bcd, const char *asc, int len, int fmt);
	static int BCD2ASC(char *asc, const unsigned char *bcd, int len, int fmt);
	static int C4toInt(unsigned char * c4);
	static int InttoC4(int integer, unsigned char * c4);
	static int bcd2int(unsigned char * bcd);
	static void int2bcd(int n, unsigned char * bcd);
	static int emvtlv_decode(uchar * ptlvstr, int tlvlen,
		tlvobject * pobjects, int objspace, int deflag);
	static int emvtlv_get(int parent, uint32 tagname,
	       tlvobject * pobjects, int level);
	
	static int emvtlv_get_next(int pos, tlvobject * pobjects, int level);
	static unsigned char calc_lrc(unsigned char *cpBuf, int iLength);

	static unsigned int ComputCrc32(unsigned int uCrc,unsigned char *Buf,int nLen);	
	static unsigned int crc32_org(unsigned int crc, unsigned char *buf, size_t len);	
	static int recur_decode(int parent,tlvobject * pobjects, int objspace);
	
	static void sha1_starts( sha1_context *ctx );
	static void sha1_process( sha1_context *ctx, _sha1_uint8 data[64] );
	static void sha1_update( sha1_context *ctx, _sha1_uint8 *input, _sha1_uint32 length );
	static void sha1_finish( sha1_context *ctx, _sha1_uint8 digest[20] );
	static unsigned long crc32(unsigned long crc, unsigned char *buf, size_t len);

	static int RSAPublicEncrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int,
		R_RSA_PUBLIC_KEY *);
	static int RSAPrivateEncrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int,
		R_RSA_PRIVATE_KEY *);
	static int RSAPublicDecrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int,
		R_RSA_PUBLIC_KEY *);
	static int RSAPrivateDecrypt (unsigned char *, unsigned int *, unsigned char *, unsigned int,
		R_RSA_PRIVATE_KEY *);
};

#endif // !defined(AFX_CODETOOLS_H__2B21C0B8_78F6_472D_9215_AC038F61D785__INCLUDED_)
