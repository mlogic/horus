
extern unsigned int block_size[];

void key_to_value (char *key, int *key_len, char *string);
void key_to_string (char *buf, int size, char *key, int key_len);

char *print_key (char *key, int key_len);

char *
block_key (char *key, int *key_len,
           char *parent, int parent_len, int x, int y);

int
horus_key_from_to (char *key, int *key_len, int x, int y,
                   char *parent, int parent_len, int parent_x, int parent_y);

int
horus_key_by_master (char *key, int *key_len, int x, int y,
                     char *master, int master_len);

void
horus_key (char *key, int *key_len, int filepos,
           char *ktype, char *kvalue);

void
horus_get_key (char **ktype, char **kvalue);


