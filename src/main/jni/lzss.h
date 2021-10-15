/*******************************************************************************
 * Describe: LZSS压缩算法，用于NLC解包以及早期ARD包的解包，现在用的ARD包采用zlib解压
 ******************************************************************************/
#ifndef __CLZSS_INCLUDED__
#define __CLZSS_INCLUDED__

#include <stdio.h>
#include "sha1.h"
#include <assert.h>
typedef FILE *  PFILE;

class CByteIn
{
#define  N 4096
private:
	unsigned char buf[N];
	int off;
	int len;
	FILE *pfin;
public:
	CByteIn()
	{
		off=0;
		len=0;
		pfin=NULL;
	}
	
	~CByteIn()
	{
	}
	
	int getc(FILE *fp)
	{
		if(pfin!=fp)
		{
			len=0;
		}
		if(off>=len)
		{
			pfin=fp;
			len=fread(buf,sizeof(unsigned char),N,fp);
			if (len == 0) 
			{
				return EOF;
			}
			off=0;
		}
		return buf[off++];
	}
#undef  N 
};

class CByteOut
{
#define  N 10240
private:
	unsigned char buf[N];
	int off;
	FILE *pfout;
public:
	CByteOut()
	{
		off=0;
	}
	
	void flush()
	{
		int len;
		if(off)
		{
			len=fwrite(buf,sizeof(unsigned char),off,pfout);
			assert(len==off);
			off=0;
		}
	}

	~CByteOut()
	{
		flush();
	}

	void putc(int c, FILE *fp)
	{
		int len;
		
		pfout=fp;
		buf[off++]=(unsigned char)c;
		if(off==N)
		{
			len=fwrite(buf,sizeof(unsigned char),N,fp);
			assert(len==N);
			off=0;
		}
	}
#undef  N 
};

class CLzss
{
        enum LZSSDATA
        {
                inMEM = 1,
                inFILE,
				inFILES
        };
public:
        CLzss();
        ~CLzss();

        unsigned int LZSSCompress( unsigned char *in, unsigned int insize, unsigned char *out );		// �ڴ��е�����ѹ��
        unsigned int LZSSDeCompress( unsigned char *in, unsigned int insize, unsigned char *out );	// �ڴ��е����ݽ�ѹ

        unsigned int Compress( unsigned char *in, unsigned int insize, FILE *out );		// ���ڴ��е�����ѹ����д���ļ�
        unsigned int DeCompress( FILE *in, unsigned int insize, unsigned char *out );

		unsigned int Compress( FILE *in, unsigned int insize, FILE *out );	// ѹ���ļ�                                                
        unsigned int DeCompress( FILE *in, unsigned int insize, FILE *out );	// ���ļ��е����ݽ�ѹ��д���ڴ�                                                        // ��ѹ�ļ�
		
		unsigned int Compress(FILE * in[], unsigned int insize, int innum, FILE * out);
		unsigned int DeCompress(FILE * in, int insize, FILE * out[], unsigned int outlen[], int outnum);

		unsigned char * GetDigest(void);
private:
        int GetByte(CByteIn *bytein);				// ��ȡһ���ֽڵ�����
        void PutByte( unsigned char c,CByteOut* byteout);	// д��һ���ֽڵ�����
        void InitTree();							// ��ʼ������
        void InsertNode( int r );					// ����һ������
        void DeleteNode( int p );					// ɾ��һ������
        void Encode();								// ѹ������
        void Decode();								// ��ѹ����

private:
        unsigned char *buffer;
        int mpos, mlen;
        int *lson, *rson, *dad;
        unsigned char InType, OutType;                        // ���������������,ָ�����ļ������ڴ��е�����
        unsigned char *InData, *OutData;                // �����������ָ��
        FILE *fpIn, *fpOut;                                                // ��������ļ�ָ��
        unsigned int InDataSize;                                // �������ݳ���
        unsigned int InSize, OutSize;                        // ������������ݳ���

		FILE ** fpIns ;
		int fpinnum, fpindex;
		unsigned char *in_filebuffer, * pin_curr, * pin_end ;

		FILE ** fpOuts;
		int fpoutnum, fpoutindex;
		unsigned int * fpoutlen;
	
		union hash_state md;
		unsigned char digest[20];

};
#endif