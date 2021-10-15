#ifndef _RSA_H_
#define _RSA_H_

#include "pubdefine.h"
/* Type definitions.
 */

#define R_memset(x, y, z) memset(x, y, z)
#define R_memcpy(x, y, z) memcpy(x, y, z)
#define R_memcmp(x, y, z) memcmp(x, y, z)

#define PROTO_LIST(list) list

#define MIN_RSA_MODULUS_BITS 508
#define MAX_RSA_MODULUS_BITS 2048
#define MAX_RSA_MODULUS_LEN ((MAX_RSA_MODULUS_BITS + 7) / 8)
#define MAX_RSA_PRIME_BITS ((MAX_RSA_MODULUS_BITS + 1) / 2)
#define MAX_RSA_PRIME_LEN ((MAX_RSA_PRIME_BITS + 7) / 8)


#define RE_LEN 0x0406
#define RE_MODULUS_LEN 0x0407
#define RE_PRIVATE_KEY 0x0409
#define RE_PUBLIC_KEY 0x040a
#define RE_DATA 0x0401


/* RSA public and private key.
 */
typedef struct 
{
	unsigned int bits;                           /* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN];                    /* modulus */
	unsigned char exponent[MAX_RSA_MODULUS_LEN];           /* public exponent */
} R_RSA_PUBLIC_KEY;

typedef struct 
{
	unsigned short int bits;                     /* length in bits of modulus */
	unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
	unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */
	unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* private exponent */
	unsigned char prime[2][MAX_RSA_PRIME_LEN];   /* prime factors */
	unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];     /* exponents for CRT */
	unsigned char coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient */
} R_RSA_PRIVATE_KEY;

#endif

