#ifndef __HASH_H__
#define __HASH_H__


#define ROL(x, y) ( ((x) << ((y) & 31)) | ((x) >> (32 - ((y) & 31))) )
#define ROR(x, y) ( ((x) >> ((y) & 31)) | ((x) << (32 - ((y) & 31))) )

struct sha1_state {
	unsigned int state[5], length, curlen;
	unsigned char buf[64];
};

union hash_state {
	struct sha1_state   sha1;
};


void sha1_init(union hash_state * md);
void sha1_process(union hash_state * md, unsigned char *buf, int len);
void sha1_done(union hash_state * md, unsigned char *hash);


#endif /* __HASH_H__ */

