
#ifndef _OPENSSL_H_
#define _OPENSSL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

EVP_PKEY *
openssl_load_private_key (char *filename, char *pass);

#endif /*_OPENSSL_H_*/

