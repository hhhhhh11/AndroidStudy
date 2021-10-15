// CodeTools.cpp: implementation of the CCodeTools class.
//
//////////////////////////////////////////////////////////////////////

#include "codetools.h"
#include <unistd.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodeTools::CCodeTools()
{

}

CCodeTools::~CCodeTools()
{

}
	

int CCodeTools::ASC2BCD(unsigned char *bcdstr, const char *ascstr, int len, int align)
{
	int i = 0;
	uchar ch;
	
	for (i = 0; i < (len + 1) / 2; i++)
		*(bcdstr + i) = 0x00;
	
	if ((len & 0x01) && align)
		align = 1;
	else
		align = 0;
	
	for (i = align; i < len + align; i ++) {
		ch = *(ascstr + i - align);
		if (ch >= 'a'&& ch <= 'f')
			ch -= ('a' - 0x0A);
		else if (ch >= 'A'&& ch <= 'F')
			ch -= ('A' - 0x0A);
		else if (ch >= '0' && ch <= '9')
			ch -= '0';
		else
			return -1;
		
		*(bcdstr + (i / 2)) |= (ch << (((i + 1) % 2) * 4));
	}
	
	return 0;
}

int CCodeTools::BCD2ASC(char *ascstr, const unsigned char *bcd, int len, int align)
{
	int             cnt;
    unsigned char  *pBcdBuf;
	char * asc = ascstr;
	
    pBcdBuf = (unsigned char *) bcd;
	
    if ((len & 0x01) && align)   /*�б��Ƿ�Ϊ�����Լ����Ǳ߶��� */
    {                           /*0��1�� */
        cnt = 1;
        len++;
    }
    else
        cnt = 0;
    for (; cnt < len; cnt++, asc++)
    {
        *asc = ((cnt & 0x01) ? (*pBcdBuf++ & 0x0f) : (*pBcdBuf >> 4));
        *asc += ((*asc > 9) ? ('A' - 10) : '0');
    }
    *asc = 0;
    return 0;
}

int CCodeTools::C4toInt(unsigned char * c4) 
{
	union trans {
		int value ;
		char littleendian[4] ;
	} tr ;
	
	tr.littleendian[0] = c4[3];
	tr.littleendian[1] = c4[2];
	tr.littleendian[2] = c4[1];
	tr.littleendian[3] = c4[0];
	
	return tr.value ;
}

int CCodeTools::InttoC4(int integer, unsigned char * c4) 
{
	union trans {
		int value ;
		char littleendian[4] ;
	} tr ;
	
	tr.value = integer ;
	c4[3] = tr.littleendian[0];
	c4[2] = tr.littleendian[1];
	c4[1] = tr.littleendian[2];
	c4[0] = tr.littleendian[3];
	
	return 0 ;
}
void CCodeTools::int2bcd(int n, unsigned char * bcd)
{
	bcd[0] = ByteToBcd((n)/100);
	bcd[1] = ByteToBcd((n)%100);
}

int CCodeTools::bcd2int(unsigned char * bcd)
{
	return (BcdToByte((bcd)[0])*100+BcdToByte((bcd)[1]));
}
static int recur_decode(int parent,
		tlvobject * pobjects, int objspace)
{
	int itrack = 0, i, tmplen;
	int tlvpos = parent + 1;
	uint32 tmptag ;
	uchar * ptlvstr = pobjects[parent].pvalue;
	int tlvlen = pobjects[parent].valuelen;

	while (itrack < tlvlen) {

		if (ptlvstr[itrack] == 0x00 ||
				ptlvstr[itrack] == 0xFF) {
			itrack ++;
			continue;
		}

		if (tlvpos == objspace)
			return NO_OBJSPACE;

		if (is_primitive(ptlvstr[itrack]))
			pobjects[tlvpos].childnum = -1;
		else
			pobjects[tlvpos].childnum = 0;

		tmptag = 0;
		if (has_subsequent(ptlvstr[itrack])) {
			i = 0;
			do {
				tmptag = (tmptag | ptlvstr[itrack ++]) << 8 ;
				if ( ++ i == 4 || itrack == tlvlen)
					return DETAG_LENERR;
			}while(another_byte_follow(ptlvstr[itrack]));
		}
		pobjects[tlvpos].tagname = tmptag | ptlvstr[itrack ++];
		if (itrack == tlvlen)
			return DETAG_LENERR;

		tmplen = ptlvstr[itrack] & 0x7F;
		if (lenlen_exceed_one(ptlvstr[itrack ++])) {
			if (tmplen > 4 || tmplen > tlvlen - itrack)
				return DELEN_LENERR;
			i = tmplen;
			tmplen = 0;
			while (i > 0)
				tmplen |= ptlvstr[itrack ++] << (--i * 8);
		}
		if (tmplen > tlvlen - itrack)
			return DELEN_LENERR;

		pobjects[tlvpos].valuelen = tmplen;
		pobjects[tlvpos].parent = parent;
		pobjects[tlvpos].pvalue = ptlvstr + itrack;
		itrack += tmplen;

		if (pobjects[tlvpos].childnum == 0) {
			if ((tmplen = recur_decode(tlvpos, pobjects, objspace)) < 0)
				return tmplen;
			tlvpos += pobjects[tlvpos].childnum;
		}
		tlvpos ++;
	}

	pobjects[parent].childnum = tlvpos - parent - 1;
	return 0;
}


int CCodeTools::emvtlv_decode(uchar * ptlvstr, int tlvlen,
		  tlvobject * pobjects, int objspace, int deflag)
{
	int ret ;

	if (ptlvstr == NULL || pobjects == NULL)
		return ILL_PARAMETER;

	if (*ptlvstr == 0x00 || *ptlvstr == 0xFF)
		return ILL_TAGSTART;

	memset(pobjects, 0x00, sizeof (tlvobject) * objspace);
	pobjects[0].pvalue = ptlvstr;
	pobjects[0].valuelen = tlvlen;
	if ((ret = recur_decode(0, pobjects, objspace)) < 0)
		return ret;

	if (deflag == SINGLE_TLVOBJ) {
		if (pobjects[1].pvalue - ptlvstr + pobjects[1].valuelen
				  != tlvlen)
			return NOT_SINGLEOBJ;
	}
	return 0;
}


int CCodeTools::emvtlv_get(int parent, uint32 tagname,
	       tlvobject * pobjects, int level)
{
	int i = (parent < 0 ? 0 : parent) + 1;
	int end ;

	if (pobjects == NULL)
		return 0;

	end = pobjects[i - 1].childnum + i;

	while (i < end) {
		if (pobjects[i].tagname == tagname)
			return i;
		if (level == SEARCH_ONLY_SON) {
			if (pobjects[i].childnum > 0) {
				i += pobjects[i].childnum;
			}
		}
		i ++;
	}
	return 0;
}

int CCodeTools::emvtlv_get_next(int pos, tlvobject * pobjects, int level)
{
	int i;
	int end;

	if (pos <= 0 || pobjects == NULL)
		return 0;

	i = pobjects[pos].parent;
	end = pobjects[i].childnum + i + 1;
	i = pos + 1;
	while (i < end) {
		if (pobjects[i].tagname == pobjects[pos].tagname)
			return i;
		if (level == SEARCH_ONLY_SON) {
			if (pobjects[i].childnum > 0) {
				i += pobjects[i].childnum;
			}
		}
		i ++;
	}
	return 0;
}

unsigned char CCodeTools::calc_lrc(unsigned char *cpBuf, int iLength)
{
	int     i;
	unsigned char   ch;
	ch = '\x00';
	for(i=0; i<iLength; i++)        
		ch ^= cpBuf[i];
	return(ch);
}

#define GET__sha1_uint32(n,b,i)                       \
{                                               \
    (n) = ( (_sha1_uint32) (b)[(i)    ] << 24 )       \
        | ( (_sha1_uint32) (b)[(i) + 1] << 16 )       \
        | ( (_sha1_uint32) (b)[(i) + 2] <<  8 )       \
        | ( (_sha1_uint32) (b)[(i) + 3]       );      \
}

#define PUT__sha1_uint32(n,b,i)                       \
{                                               \
    (b)[(i)    ] = (_sha1_uint8) ( (n) >> 24 );       \
    (b)[(i) + 1] = (_sha1_uint8) ( (n) >> 16 );       \
    (b)[(i) + 2] = (_sha1_uint8) ( (n) >>  8 );       \
    (b)[(i) + 3] = (_sha1_uint8) ( (n)       );       \
}

void CCodeTools::sha1_starts( sha1_context *ctx )
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;
}

void CCodeTools::sha1_process( sha1_context *ctx, _sha1_uint8 data[64] )
{
	_sha1_uint32 temp, W[16], A, B, C, D, E;

	GET__sha1_uint32( W[0],  data,  0 );
	GET__sha1_uint32( W[1],  data,  4 );
	GET__sha1_uint32( W[2],  data,  8 );
	GET__sha1_uint32( W[3],  data, 12 );
	GET__sha1_uint32( W[4],  data, 16 );
	GET__sha1_uint32( W[5],  data, 20 );
	GET__sha1_uint32( W[6],  data, 24 );
	GET__sha1_uint32( W[7],  data, 28 );
	GET__sha1_uint32( W[8],  data, 32 );
	GET__sha1_uint32( W[9],  data, 36 );
	GET__sha1_uint32( W[10], data, 40 );
	GET__sha1_uint32( W[11], data, 44 );
	GET__sha1_uint32( W[12], data, 48 );
	GET__sha1_uint32( W[13], data, 52 );
	GET__sha1_uint32( W[14], data, 56 );
	GET__sha1_uint32( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
)

#define P(a,b,c,d,e,x)                                  \
{                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

	P( A, B, C, D, E, W[0]  );
	P( E, A, B, C, D, W[1]  );
	P( D, E, A, B, C, W[2]  );
	P( C, D, E, A, B, W[3]  );
	P( B, C, D, E, A, W[4]  );
	P( A, B, C, D, E, W[5]  );
	P( E, A, B, C, D, W[6]  );
	P( D, E, A, B, C, W[7]  );
	P( C, D, E, A, B, W[8]  );
	P( B, C, D, E, A, W[9]  );
	P( A, B, C, D, E, W[10] );
	P( E, A, B, C, D, W[11] );
	P( D, E, A, B, C, W[12] );
	P( C, D, E, A, B, W[13] );
	P( B, C, D, E, A, W[14] );
	P( A, B, C, D, E, W[15] );
	P( E, A, B, C, D, R(16) );
	P( D, E, A, B, C, R(17) );
	P( C, D, E, A, B, R(18) );
	P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

	P( A, B, C, D, E, R(20) );
	P( E, A, B, C, D, R(21) );
	P( D, E, A, B, C, R(22) );
	P( C, D, E, A, B, R(23) );
	P( B, C, D, E, A, R(24) );
	P( A, B, C, D, E, R(25) );
	P( E, A, B, C, D, R(26) );
	P( D, E, A, B, C, R(27) );
	P( C, D, E, A, B, R(28) );
	P( B, C, D, E, A, R(29) );
	P( A, B, C, D, E, R(30) );
	P( E, A, B, C, D, R(31) );
	P( D, E, A, B, C, R(32) );
	P( C, D, E, A, B, R(33) );
	P( B, C, D, E, A, R(34) );
	P( A, B, C, D, E, R(35) );
	P( E, A, B, C, D, R(36) );
	P( D, E, A, B, C, R(37) );
	P( C, D, E, A, B, R(38) );
	P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

	P( A, B, C, D, E, R(40) );
	P( E, A, B, C, D, R(41) );
	P( D, E, A, B, C, R(42) );
	P( C, D, E, A, B, R(43) );
	P( B, C, D, E, A, R(44) );
	P( A, B, C, D, E, R(45) );
	P( E, A, B, C, D, R(46) );
	P( D, E, A, B, C, R(47) );
	P( C, D, E, A, B, R(48) );
	P( B, C, D, E, A, R(49) );
	P( A, B, C, D, E, R(50) );
	P( E, A, B, C, D, R(51) );
	P( D, E, A, B, C, R(52) );
	P( C, D, E, A, B, R(53) );
	P( B, C, D, E, A, R(54) );
	P( A, B, C, D, E, R(55) );
	P( E, A, B, C, D, R(56) );
	P( D, E, A, B, C, R(57) );
	P( C, D, E, A, B, R(58) );
	P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

	P( A, B, C, D, E, R(60) );
	P( E, A, B, C, D, R(61) );
	P( D, E, A, B, C, R(62) );
	P( C, D, E, A, B, R(63) );
	P( B, C, D, E, A, R(64) );
	P( A, B, C, D, E, R(65) );
	P( E, A, B, C, D, R(66) );
	P( D, E, A, B, C, R(67) );
	P( C, D, E, A, B, R(68) );
	P( B, C, D, E, A, R(69) );
	P( A, B, C, D, E, R(70) );
	P( E, A, B, C, D, R(71) );
	P( D, E, A, B, C, R(72) );
	P( C, D, E, A, B, R(73) );
	P( B, C, D, E, A, R(74) );
	P( A, B, C, D, E, R(75) );
	P( E, A, B, C, D, R(76) );
	P( D, E, A, B, C, R(77) );
	P( C, D, E, A, B, R(78) );
	P( B, C, D, E, A, R(79) );

#undef K
#undef F

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;
}

void CCodeTools::sha1_update( sha1_context *ctx, _sha1_uint8 *input, _sha1_uint32 length )
{
	_sha1_uint32 left, fill;

	if ( ! length ) return;

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += length;
	ctx->total[0] &= 0xFFFFFFFF;

	if ( ctx->total[0] < length )
		ctx->total[1]++;

	if ( left && length >= fill ) {
		memcpy( (void *) (ctx->buffer + left),
		        (void *) input, fill );
		sha1_process( ctx, ctx->buffer );
		length -= fill;
		input  += fill;
		left = 0;
	}

	while ( length >= 64 ) {
		sha1_process( ctx, input );
		length -= 64;
		input  += 64;
	}

	if ( length ) {
		memcpy( (void *) (ctx->buffer + left),
		        (void *) input, length );
	}
}

static _sha1_uint8 sha1_padding[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void CCodeTools::sha1_finish( sha1_context *ctx, _sha1_uint8 digest[20] )
{
	_sha1_uint32 last, padn;
	_sha1_uint32 high, low;
	_sha1_uint8 msglen[8];

	high = ( ctx->total[0] >> 29 )
	       | ( ctx->total[1] <<  3 );
	low  = ( ctx->total[0] <<  3 );

	PUT__sha1_uint32( high, msglen, 0 );
	PUT__sha1_uint32( low,  msglen, 4 );

	last = ctx->total[0] & 0x3F;
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

	sha1_update( ctx, sha1_padding, padn );
	sha1_update( ctx, msglen, 8 );

	PUT__sha1_uint32( ctx->state[0], digest,  0 );
	PUT__sha1_uint32( ctx->state[1], digest,  4 );
	PUT__sha1_uint32( ctx->state[2], digest,  8 );
	PUT__sha1_uint32( ctx->state[3], digest, 12 );
	PUT__sha1_uint32( ctx->state[4], digest, 16 );
}

static unsigned long crc_table[] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
		0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
		0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
		0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
		0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
		0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
		0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
		0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
		0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
		0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
		0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
		0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
		0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
		0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
		0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
		0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
		0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
		0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
		0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
		0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
		0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
		0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
		0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
		0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
		0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
		0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
		0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
		0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
		0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
		0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
		0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
		0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
		0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
		0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
		0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
		0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
		0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
		0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
		0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
		0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
		0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
		0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
		0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
		0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
		0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
		0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
		0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
		0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
		0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
		0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
		0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
		0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf) DO1(buf); DO1(buf);
#define DO4(buf) DO2(buf); DO2(buf);
#define DO8(buf) DO4(buf); DO4(buf);

/* ========================================================================= */
unsigned long CCodeTools::crc32(unsigned long crc, unsigned char *buf, size_t len)
{
	if (buf == NULL)
		return 0L;
	
	crc = crc ^ 0xffffffffL;
	while (len >= 8) {
		DO8(buf);
		len -= 8;
	}
	if (len) do {
		DO1(buf);
	} while (--len);
	return crc ^ 0xffffffffL;
}


unsigned int CCodeTools::crc32_org(unsigned int crc, unsigned char *buf, size_t len)
{
	if (buf == NULL)
		return 0L;
	
	crc = crc ^ 0xffffffffL;
	while (len >= 8) {
		DO8(buf);
		len -= 8;
	}
	if (len) do {
		DO1(buf);
	} while (--len);
	return crc ^ 0xffffffffL;
}

	
int CCodeTools::recur_decode(int parent,tlvobject * pobjects, int objspace)
	{
		int itrack = 0, i, tmplen;
		int tlvpos = parent + 1;
		unsigned int tmptag ;
		unsigned char * ptlvstr = pobjects[parent].pvalue;
		int tlvlen = pobjects[parent].valuelen;
		
		while (itrack < tlvlen) 
		{
			
			if (ptlvstr[itrack] == 0x00 ||
				ptlvstr[itrack] == 0xFF) 
			{
				itrack ++;
				continue;
			}
			
			if (tlvpos == objspace)
				return NO_OBJSPACE;
			
			if (is_primitive(ptlvstr[itrack]))
				pobjects[tlvpos].childnum = -1;
			else
				pobjects[tlvpos].childnum = 0;
			
			tmptag = 0;
			if (has_subsequent(ptlvstr[itrack])) 
			{
				i = 0;
				do 
				{
					tmptag = (tmptag | ptlvstr[itrack ++]) << 8 ;
					
					if ( ++ i == 4 || itrack == tlvlen)
						return DETAG_LENERR;
						
				}while(another_byte_follow(ptlvstr[itrack]));
			}
			pobjects[tlvpos].tagname = tmptag | ptlvstr[itrack ++];
			if (itrack == tlvlen)
				return DETAG_LENERR;
			
			tmplen = ptlvstr[itrack] & 0x7F;
			if (lenlen_exceed_one(ptlvstr[itrack ++])) 
			{
				if (tmplen > 4 || tmplen > tlvlen - itrack)
					return DELEN_LENERR;
					
				i = tmplen;
				tmplen = 0;
				
				while (i > 0)
					tmplen |= ptlvstr[itrack ++] << (--i * 8);
			}
			if (tmplen > tlvlen - itrack)
				return DELEN_LENERR;
			
			pobjects[tlvpos].valuelen = tmplen;
			pobjects[tlvpos].parent = parent;
			pobjects[tlvpos].pvalue = ptlvstr + itrack;
			itrack += tmplen;
			
			if (pobjects[tlvpos].childnum == 0) 
			{
				if ((tmplen = recur_decode(tlvpos, pobjects, objspace)) < 0)
					return tmplen;
				tlvpos += pobjects[tlvpos].childnum;
			}
			tlvpos ++;
		}
		
		pobjects[parent].childnum = tlvpos - parent - 1;
		return 0;
	}

unsigned int CCodeTools::ComputCrc32(unsigned int uCrc,unsigned char *Buf,int nLen)
{
		int i;
		unsigned int  ulTmpcrc = uCrc^0xffffffff;
	
		for (i=0;i<nLen;i++)
		{
			ulTmpcrc=(ulTmpcrc >> 8)^crc_table[(ulTmpcrc & 0xff) ^ *Buf++];
	
		}
	  
		return ulTmpcrc ^ 0xffffffff;  
	
}

