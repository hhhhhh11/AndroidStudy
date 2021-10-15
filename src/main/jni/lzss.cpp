
#include "lzss.h"

#define N               4096
#define F               18
#define THRESHOLD       2
#define NIL             N




CLzss:: CLzss()
{
        buffer = new unsigned char[N+F-1];
        lson = new int[N+1];
        rson = new int[N+257];
        dad = new int[N+1];
		fpIns = NULL;
		fpinnum = 0;
		in_filebuffer = new unsigned char[N];
		pin_curr = pin_end = NULL;

		fpOuts = NULL;
		fpoutnum = 0;
		fpoutlen = NULL;
}

CLzss:: ~CLzss()
{
        delete buffer;
        delete lson;
        delete rson;
        delete dad;
		delete in_filebuffer;
}

int CLzss:: GetByte(CByteIn *bytein=NULL)
{
		int len;
        if( InSize++ >= InDataSize )
                return( EOF );

        switch( InType )
        {
                case inMEM:
                        return( *InData++ );

                case inFILE:
						if(bytein)
							return bytein->getc(fpIn);
						else
							return( getc( fpIn ) );
				
				case inFILES:
					if (pin_curr == pin_end) {
						len = fread(in_filebuffer, sizeof(unsigned char), N, fpIn);
						while (len == 0 && fpindex < fpinnum - 1) {
							fpIn = fpIns[++fpindex];
							len = fread(in_filebuffer, sizeof(unsigned char), N, fpIn);
						}
						if (len == 0) {
							return EOF;
						}
						sha1_process(&md, in_filebuffer, len);
						pin_curr = in_filebuffer + 1, pin_end = in_filebuffer + len;
						return in_filebuffer[0];
					}
					return *pin_curr ++;				
        }

        return(EOF);
}

void CLzss:: PutByte( unsigned char c ,CByteOut *byteout=NULL)
{
        OutSize++;

        switch( OutType )
        {
                case inMEM:
                        *OutData++=c;
                        return;

                case inFILE:
						if(byteout)
							byteout->putc(c,fpOut);
						else
							putc(c,fpOut);
                        return;
				
				case inFILES:
					if(byteout)
						byteout->putc(c,fpOut);
					else
						putc(c,fpOut);
					if (pin_curr == pin_end)
					{
						sha1_process(&md, in_filebuffer, N);
						pin_curr = in_filebuffer;
					}
					*pin_curr ++ = (char)c;
					if (OutSize == fpoutlen[fpoutindex]) {
						if(byteout)byteout->flush();
						if (fpoutindex < fpoutnum - 1) {
							fpOut = fpOuts[++fpoutindex];
							OutSize = 0;
						}
					}
					return;
        }
}

void CLzss:: InitTree()
{
        int i;

        for(i=N+1; i<=N+256; i++)
                rson[i]=NIL;

        for(i=0; i<N; i++)
                dad[i]=NIL;
}

void CLzss:: InsertNode( int r )
{
        int i, p, cmp;
        unsigned char *key;
        cmp = 1;
        key = &buffer[r];
        p = N+1+key[0];
        rson[r] = NIL;
        lson[r] = NIL;
        mlen=0;

        while(1)
        {
                if( cmp >= 0 )
                {
                        if( rson[p] != NIL )
                                p=rson[p];
                        else
                        {
                                rson[p] = r;
                                dad[r] = p;
                                return;
                        }
                }
                else
                {
                        if(lson[p] != NIL )
                                p=lson[p];
                        else
                        {
                                lson[p]=r;
                                dad[r]=p;
                                return;
                        }
                }

                for( i=1; i<F; i++ )
                {
                        cmp = key[i] - buffer[p+i];

                        if( cmp != 0 )
                                break;
                }

                if( i > mlen )
                {
                        mpos = p;
                        mlen = i;
                        if( mlen >= F )
                                break;
                }
        }
        
        dad[r] = dad[p];
        lson[r] = lson[p];
        rson[r] = rson[p];
        dad[lson[p]] = r;
        dad[rson[p]] = r;

        if( rson[dad[p]] == p )
                rson[dad[p]] = r;
        else 
                lson[dad[p]] = r;

        dad[p] = NIL;
}

void CLzss:: DeleteNode( int p )
{
        int q;

        if( dad[p] == NIL )
                return;
        
        if( rson[p] == NIL )
                q = lson[p];
        else if( lson[p] == NIL )
                q = rson[p];
        else
        {
                q = lson[p];
                if( rson[q] != NIL )
                {
                        do
                        {
                                q = rson[q];
                        }while( rson[q] != NIL );

                        rson[dad[q]] = lson[q];
                        dad[lson[q]] = dad[q];
                        lson[q] = lson[p];
                        dad[lson[p]] = q;
                }
                
                rson[q] = rson[p];
                dad[rson[p]] = q;
        }

        dad[q] = dad[p];
        if( rson[dad[p]] == p )
                rson[dad[p]] = q;
    else 
                lson[dad[p]] = q;
        
        dad[p] = NIL;
}

void CLzss:: Encode()
{
        int i, c, len, r, s, lml, cbp;
        unsigned char codebuf[17], mask;
        InitTree();
        codebuf[0]=0;
        cbp=1;
        mask=1;
        s=0;
        r=N-F;
        for(i=s; i<r; i++)
                buffer[i]=' ';

        for(len=0; len<F && (c=GetByte())!=EOF; len++)
                buffer[r+len]=c;

        if(len==0)
                return;

        for(i=1;i<=F;i++)
                InsertNode(r-i);
        
        InsertNode(r);
        
        do
        {
                if(mlen>len)
                        mlen=len;
                if(mlen<=THRESHOLD)
                {
                        mlen=1;
                        codebuf[0]|=mask;
                        codebuf[cbp++]=buffer[r];
                }
                else
                {
                        codebuf[cbp++]=(unsigned char)mpos;
                        codebuf[cbp++]=(unsigned char)(((mpos>>4)&0xF0)|(mlen-(THRESHOLD+1)));
                }

                if((mask<<=1)==0)
                {
                        for(i=0; i<cbp; i++)
                                PutByte(codebuf[i]);

                        codebuf[0]=0;
                        cbp=1;
                        mask=1;
                }

                lml=mlen;
                for(i=0; i<lml&&(c=GetByte())!=EOF; i++)
                {
                        DeleteNode(s);
                        buffer[s]=c;
                        if(s<F-1)
                                buffer[s+N]=c;
                        s=(s+1)&(N-1);
                        r=(r+1)&(N-1);
                        InsertNode(r);
                }

                while(i++<lml)
                {
                        DeleteNode(s);
                        s=(s+1)&(N-1);
                        r=(r+1)&(N-1);
                        if(--len)
                                InsertNode(r);
                }
        }while(len>0);

        if(cbp>1)
        {
                for(i=0; i<cbp; i++)
                        PutByte(codebuf[i]);
        }
}

void CLzss:: Decode()
{
        int i, j, k, r, c;
        unsigned int flags;
		CByteOut byteout;
        CByteIn bytein;

        for(i=0; i<N-F; i++)
                buffer[i]=' ';
        
        r=N-F;
        flags=0;

        for(;;)
        {
                if(((flags>>=1)&256)==0)
                {
                        if((c=GetByte(&bytein))==EOF)
                                break;

                        flags=c|0xFF00;
                }

                if(flags&1)
                {
                        if((c=GetByte(&bytein))==EOF)
                                break;
                        
                        PutByte(c,&byteout);
                        buffer[r++]=c;
                        r&=(N-1);
                }
                else
                {
                        if((i=GetByte(&bytein))==EOF)
                                break;

                        if((j=GetByte(&bytein))==EOF)
                                break;
                        
                        i|=((j&0xF0)<<4);
                        j=(j&0x0F)+THRESHOLD;
                        
                        for(k=0;k<=j;k++)
                        {
                                c=buffer[(i+k)&(N-1)];
                                PutByte(c,&byteout);
                                buffer[r++]=c;
                                r&=(N-1);
                        }
                }
        }
}

unsigned int CLzss:: LZSSCompress( unsigned char *in, unsigned int insize, unsigned char *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inMEM;
    InData=in;
    InDataSize=insize;
    InSize=0;

    OutType=inMEM;
    OutData=out;
    OutSize=0;

    Encode();
    
    return(OutSize);
}

unsigned int CLzss:: Compress( unsigned char *in, unsigned int insize, FILE *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inMEM;
    InData=in;
    InDataSize=insize;
    InSize=0;

    OutType=inFILE;
    fpOut=out;
    OutSize=0;

    Encode();

    return(OutSize);
}

unsigned int CLzss:: Compress( FILE *in, unsigned int insize, FILE *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inFILE;
    fpIn=in;
    InDataSize=insize;
    InSize=0;

    OutType=inFILE;
    fpOut=out;
    OutSize=0;
    
    Encode();

    return(OutSize);
}

unsigned int CLzss:: LZSSDeCompress( unsigned char *in, unsigned int insize, unsigned char *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inMEM;
    InData=in;
    InDataSize=insize;
    InSize=0;

    OutType=inMEM;
    OutData=out;
    OutSize=0;

    Decode();

    return(OutSize);
}

unsigned int CLzss:: DeCompress( FILE *in, unsigned int insize, unsigned char *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inFILE;
    fpIn=in;
    InDataSize=insize;
    InSize=0;

    OutType=inMEM;
    OutData=out;
    OutSize=0;

    Decode();

    return(OutSize);
}

unsigned int CLzss:: DeCompress( FILE *in, unsigned int insize, FILE *out )
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inFILE;
    fpIn=in;
    InDataSize=insize;
    InSize=0;

    OutType=inFILE;
    fpOut=out;
    OutSize=0;

    Decode();

    return(OutSize);
}


unsigned int CLzss::Compress(FILE * in[], unsigned int insize, int innum, FILE * out)
{
	if (innum <= 0 || in == NULL || out == NULL) {
		return 0;
	}
	InType = inFILES;
	fpindex = 0;
	fpIn = in[fpindex];
	fpIns = new PFILE[innum];
	fpinnum = innum;
	InDataSize = insize;
	for (int i = 0; i < innum; i ++) {
		fpIns[i] = in[i];
	}
	InSize = 0;

	OutType = inFILE;
	fpOut = out;
	OutSize = 0;
	
	pin_curr = pin_end = NULL;
	sha1_init(&md);
	Encode();
	sha1_done(&md, digest);
	delete fpIns;
	fpIns = NULL;
	return OutSize;
}

unsigned int CLzss::DeCompress(FILE * in, int insize, FILE * out[], unsigned int outlen[], int outnum)
{
	if (in == NULL || out == NULL) {
		return 0;
	}
    InType=inFILE;
    fpIn=in;
    InDataSize=insize;
    InSize=0;
	
    OutType=inFILES;
	fpoutindex = 0;
    fpOut=out[fpoutindex];
	fpoutnum = outnum;
	
	fpOuts = new PFILE[fpoutnum];
	
	fpoutlen = new unsigned int[fpoutnum];

	for (int i = 0; i < outnum; i ++) {
		fpOuts[i] = out[i];
		fpoutlen[i] = outlen[i];
	}
    OutSize=0;
	
	pin_curr =  in_filebuffer;
	pin_end = in_filebuffer + N;
	sha1_init(&md);
    Decode();
	
 	if (pin_curr > in_filebuffer)
 		sha1_process(&md, in_filebuffer, pin_curr - in_filebuffer);
	sha1_done(&md, digest);
	
	delete fpOuts;
	delete fpoutlen;
	fpOuts = NULL;
	fpoutlen = NULL;

    return(OutSize);
}

unsigned char * CLzss::GetDigest(void)
{
	if (OutType == inFILES || InType== inFILES) {
		return digest;
	}
	return NULL;
}