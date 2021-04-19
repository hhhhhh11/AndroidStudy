#ifndef DIGEST_H_
#define DIGEST_H_

#include <stdint.h>
int  sha1_starts(void);
void sha1_update(uint8_t *input, uint32_t ilen);
void sha1_finish(uint8_t output[20]);

int  sha256_starts(void);
void sha256_update(uint8_t * input, uint32_t length);
void sha256_finish(uint8_t digest[32]);

void sm3_starts(void);
void sm3_update(uint8_t *input, uint32_t ilen );
void sm3_finish(uint8_t output[32]);

int  md5_starts(void);
void md5_update(uint8_t *input, uint32_t length);
void md5_finish(uint8_t digest[16] );

#endif /* DIGEST_H_ */
