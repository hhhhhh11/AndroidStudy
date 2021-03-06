
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>

/*******************************************************************
						DES ????
********************************************************************/
#define uchar unsigned char

#define	ENCRYPT		0		/*????*/
#define DISCRYPT    1		/*????*/

#define PARAERR	(0x1FF)
#define FAIL (-1)
#define SUCC 0
int ErrorCode;

static  unsigned char Kn[16][48];
static  unsigned char input[64], /* data input */
output[64]; /* data output */
static  unsigned char Key[64];
/*----------------------------------------------------------------

                TABLES DE CHIFFREMENT

-----------------------------------------------------------------*/


/* table 1 : permutation initiale */
static  unsigned char T1[] = {
    57,49,41,33,25,17, 9, 1,
    59,51,43,35,27,19,11, 3,
    61,53,45,37,29,21,13, 5,
    63,55,47,39,31,23,15, 7,
    56,48,40,32,24,16, 8, 0,
    58,50,42,34,26,18,10, 2,
    60,52,44,36,28,20,12, 4,
    62,54,46,38,30,22,14, 6
};

/* table 2 : permutation finale */
static  unsigned char T2[] = {
    39, 7,47,15,55,23,63,31,
    38, 6,46,14,54,22,62,30,
    37, 5,45,13,53,21,61,29,
    36, 4,44,12,52,20,60,28,
    35, 3,43,11,51,19,59,27,
    34, 2,42,10,50,18,58,26,
    33, 1,41, 9,49,17,57,25,
    32, 0,40, 8,48,16,56,24
};

/* table 3 : fonction d'expansion E  ( valeur - 1 ) */
static  unsigned char T3[] = {
    31, 0, 1, 2, 3, 4,
    3, 4, 5, 6, 7, 8,
    7, 8, 9,10,11,12,
    11,12,13,14,15,16,
    15,16,17,18,19,20,
    19,20,21,22,23,24,
    23,24,25,26,27,28,
    27,28,29,30,31, 0
};

/* table 5 : fonction de permutation P */
static  unsigned char T5[] = {
    15, 6,19,20,
    28,11,27,16,
    0,14,22,25,
    4,17,30, 9,
    1, 7,23,13,
    31,26, 2, 8,
    18,12,29, 5,
    21,10, 3,24
};

/* table 7 : table de choix 1 */
static  unsigned char T7_1_2[56] = {
    56,48,40,32,24,16, 8,
    0,57,49,41,33,25,17,
    9, 1,58,50,42,34,26,
    18,10, 2,59,51,43,35,

    62,54,46,38,30,22,14,
    6,61,53,45,37,29,21,
    13, 5,60,52,44,36,28,
    20,12, 4,27,19,11, 3
};

/* table 8 : table de d\202calage */
//  code unsigned char T8[16] =
static  unsigned char T8[] = {
    0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0
};

/* table 9 : table de choix 2 */
static  unsigned char T9[] = {
    13,16,10,23, 0, 4,
    2,27,14, 5,20, 9,
    22,18,11, 3,25, 7,
    15, 6,26,19,12, 1,
    40,51,30,36,46,54,
    29,39,50,44,32,47,
    43,48,38,55,33,52,
    45,41,49,35,28,31
};

/* table 6 : s\202lection de fonctions S1 \205 S8 */
//  code unsigned char T6[8][64] =
static  unsigned char T6[][64] = {
    /* S1 */
    {
        14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
        0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
        4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
        15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13
    },

    /* S2 */
    {
        15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
        3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
        0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
        13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9
    },

    /* S3 */
    {
        10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
        13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
        13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
        1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12
    },

    /* S4 */
    {
        7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
        13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
        10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
        3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14
    },

    /* S5 */
    {
        2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
        14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
        4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
        11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3
    },

    /* S6 */
    {
        12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
        10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
        9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
        4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13
    },

    /* S7 */
    {
        4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
        13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
        1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
        6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12
    },

    /* S8 */
    {
        13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
        1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
        7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
        2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11
    }
};

/* Table pour eclatement des valeurs precedentes */
//code unsigned char TE[16][4] =
static  unsigned char TE[][4] = {
    {0,0,0,0},
    {0,0,0,1},
    {0,0,1,0},
    {0,0,1,1},
    {0,1,0,0},
    {0,1,0,1},
    {0,1,1,0},
    {0,1,1,1},
    {1,0,0,0},
    {1,0,0,1},
    {1,0,1,0},
    {1,0,1,1},
    {1,1,0,0},
    {1,1,0,1},
    {1,1,1,0},
    {1,1,1,1}
};

/*---------------------------------------------------------------
                          CLES
---------------------------------------------------------------*/
/*----------------------------------------------------------------

   Figure 3. Key Schedule Calculation.

-----------------------------------------------------------------*/
static void Ks(uchar *Key, uchar Kn[16][48])
{
    uchar cd[56];
    uchar zt[60] ;

    int n;
    unsigned char tmp11, tmp12, tmp21, tmp22;
    int i;
    unsigned char *Knn;

    /* choix 1 */
    for (i = 0; i < 56; i++) {
        cd[i] = Key[T7_1_2[i]];
    }

    for (n = 0; n < 16; n++) {
        /* rotation \205 gauche du vecteur en fonction de l'indice */
        if (T8[n] == 0) {
            tmp11 = cd[0];
            tmp21 = cd[28];
            memcpy( zt , &cd[1] , 55 );
            memcpy( cd , zt     , 55 );
            //   memmove(&cd[0], &cd[1], 55); /* ce qui est en 1 va en0 */
            cd[27] = tmp11;
            cd[55] = tmp21;
        } else {
            tmp11 = cd[0];
            tmp12 = cd[1];
            tmp21= cd[28];
            tmp22 = cd[29];

            memcpy( zt , &cd[2] , 54 );
            memcpy( cd , zt     , 54 );

            //                    memmove(&cd[0], &cd[2], 54); /* ce qui est en 2 va en 0*/

            cd[26] = tmp11;
            cd[27] = tmp12;
            cd[54] = tmp21;
            cd[55] = tmp22;
        }
        /* choix 2 */
        Knn = Kn[n];
        for (i = 0; i < 48; i++) {
            Knn[i] = cd[T9[i]];
        }
    }
}

/*----------------------------------------------------------------

         Figure 2. Calculation of f(R, K)

-----------------------------------------------------------------*/
static  void fonction(uchar *Knn, uchar *r, uchar *s)
{
    /* n est l'indice de 1 a 16 pour choisir la cle
       r est R.
       s est le r\202sultat. */

    unsigned char x[32];
    unsigned long *px;
    int i, l;
    unsigned char c;
    unsigned char t;

    /* fonction E */    /*  + OU exclusif */
    /* S\202lection Sn */
    for (i = 0, l = 0, px = (unsigned long *) x; i < 8;) {
        c = 32 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 8 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 4 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 2 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 1 * (r[T3[l]] ^ Knn[l]);
        l++;
        c += 16 * (r[T3[l]] ^ Knn[l]);
        l++;
        /* extraction de la valeur */
        t = T6[i][c];
        i++;
        /* Eclatement de la valeur; 4 bits -> 4 bytes */
        *px = *(long *)TE[t];
        px++;
    }
    /* fonction P */
    for (i = 0; i < 32; i++) {
        s[i] = x[T5[i]];
    }
}

/*----------------------------------------------------------------
 Permutations initiale et finale
-----------------------------------------------------------------*/
static  void permutation(uchar *org, uchar *tab)
{
    unsigned char tmp[64];
    int i;


    memcpy(tmp, org, 64);
    for (i = 0; i < 64; i++) {
        org[i] = tmp[tab[i]];
    }
}
/*----------------------------------------------------------------

       Figure 1. Enciphering Computation.

-----------------------------------------------------------------*/
static  void chiffrement(uchar *xi, uchar *xo, unsigned char Kn[16][48])
{
    unsigned char r[32], l[32];
    unsigned char rp[32], lp[32];

    int i;
    int n;

    memcpy(l, &xi[0], 32);
    memcpy(r, &xi[32], 32);

    for (n = 0; n < 16; n++) {
        memcpy(lp, r, 32);

        fonction(Kn[n], r, rp);
        for (i = 0; i < 32; i++) {
            r[i] =( ( l[i]) ^ (rp[i] )  ) ;
        }
        memcpy(l, lp, 32);
    }
    memcpy(&xo[0], r, 32);
    memcpy(&xo[32], l, 32);

}


/*----------------------------------------------------------------

       Deciphering Computation.
-----------------------------------------------------------------*/

static  void dechiffrement(uchar *xi, uchar *xo, unsigned char Kn[16][48])
{
    unsigned char r[32], l[32], rp[32], lp[32];

    int i;
    int n;

    memcpy(l, &xi[0], 32);
    memcpy(r, &xi[32], 32);

    for (n = 0; n < 16; n++) {
        memcpy(lp, r, 32);
        fonction(Kn[15 - n], r, rp);
        for (i = 0; i < 32; i++) {
            r[i] =( ( l[i] ) ^ ( rp[i] )) ;
        }
        memcpy(l, lp, 32);
    }

    memcpy(&xo[0], r, 32);
    memcpy(&xo[32], l, 32);
}

/*----------------------------------------------------------------
  Eclater 64 bits en 64 octets
-----------------------------------------------------------------*/
static  void eclater(uchar *buf_bit, uchar *byte)
{
    int i;
    unsigned char m;

    for (i = 0; i < 8; i++) {
        for (m = 0x80; m != 0;  ) { // m >>= 1)
            if ((buf_bit[i] & m) != 0)
                *byte = 1;
            else
                *byte = 0;
            byte++;
            m=m/2 ;
        }
    }

}
/*----------------------------------------------------------------
  Compacter 64 octets en 64 bits
-----------------------------------------------------------------*/
static  void compacter(uchar *byte, uchar *buf_bit)
{
    int i;
    unsigned char m, n;

    for (i = 0; i < 8; i++) {
        n = 0;
        for (m = 0x80; m != 0; ) { //  m >>= 1)
            if (*byte++)
                n = n | m;
            m=m/2 ;

        }
        buf_bit[i] = n;
    }
}



/*----------------------------------------------------------------
  D E S
-----------------------------------------------------------------*/

void  zwdes(uchar *binput, uchar *boutput, uchar *bkey)
{

    eclater(binput, input);
    eclater(bkey, Key);
    Ks(Key, Kn);

    permutation(input, T1);
    chiffrement(input, output, Kn);

    permutation(output, T2);
    compacter(output, boutput);
    return;
}

void  zwundes(uchar *binput, uchar *boutput, uchar *bkey)
{

    eclater(binput, input);
    eclater(bkey, Key);
    Ks(Key, Kn);

    permutation(input, T1);
    dechiffrement(input, output, Kn);
    permutation(output, T2);
    compacter(output, boutput);

}

void  des3(uchar *binput, uchar *boutput, uchar *bkey)
{
    uchar tmpibuf[8],tmpobuf[8],tmpkey[16];
    memcpy(tmpibuf,binput,8);
    memcpy(tmpkey,bkey,16);
    zwdes(tmpibuf,tmpobuf,tmpkey);
    zwundes(tmpobuf,tmpibuf,tmpkey+8);
    zwdes(tmpibuf,tmpobuf,tmpkey);
    memcpy(boutput,tmpobuf,8);
}

void zwundes3(uchar *binput, uchar *boutput, uchar *bkey)
{
    uchar tmpibuf[8],tmpobuf[8],tmpkey[16];
    memcpy(tmpibuf,binput,8);
    memcpy(tmpkey,bkey,16);
    zwundes(tmpibuf,tmpobuf,tmpkey);
    zwdes(tmpobuf,tmpibuf,tmpkey+8);
    zwundes(tmpibuf,tmpobuf,tmpkey);
    memcpy(boutput,tmpobuf,8);
}

//?ײ??ṩ??DES???߷ǹؼ????ݵļӽ????ٶ?
int mdes(int direction, uchar *ibuf, uchar *obuf , uchar *ikey)
{
    if ((obuf == NULL) || (ibuf == NULL) || (ikey == NULL)){//????NULL?жϷ?ʽ??yanm 2011-04-13
    	return -1;	
    }

    if (direction==ENCRYPT)
        zwdes(ibuf,obuf,ikey);
    else if (direction==DISCRYPT)
        zwundes(ibuf,obuf,ikey);
    else {
        ErrorCode=PARAERR;
        return FAIL;
    }
    return SUCC;
}

int mdes3(int direction, uchar *ibuf, uchar *obuf , uchar *ikey)
{
    if ((obuf == NULL) || (ibuf == NULL) || (ikey == NULL)){//????NULL?жϷ?ʽ??yanm 2011-04-13
    	return -1;
    }

    if (direction==ENCRYPT)
        des3(ibuf,obuf,ikey);
    else if (direction==DISCRYPT)
        zwundes3(ibuf,obuf,ikey);
    else {
        ErrorCode=PARAERR;
        return FAIL;
    }
    return SUCC;
}