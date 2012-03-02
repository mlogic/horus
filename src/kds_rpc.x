const MAXNAMELEN = 4096;

struct key_request {
  string filename<MAXNAMELEN>;
  int x;
  int y;
};

struct key_rtn {
  string key<100>;
  int err;
};

program KDS_RPC {
  version KEYREQVERS {
    struct key_rtn KEYREQ(struct key_request) = 1;
  } = 1;
} = 22855;
