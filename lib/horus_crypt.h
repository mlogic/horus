
#ifndef _HORUS_CRYPT_H_
#define _HORUS_CRYPT_H_

void
horus_encrypt (char *buf, ssize_t size, unsigned long long offset);
void
horus_decrypt (char *buf, ssize_t size, unsigned long long offset);

#endif /*_HORUS_CRYPT_H_*/

