
void
horus_coding (int fd, int fdpos, char *buf, size_t nbyte,
              char *ktype, char *kvalue);

void
horus_decrypt (int fd, int fdpos, char *buf, size_t nbyte,
               char *ktype, char *kvalue);
void
horus_encrypt (int fd, int fdpos, char *buf, size_t nbyte,
               char *ktype, char *kvalue);

