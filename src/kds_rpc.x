const MAXNAMELEN = 4096;
const MAXKEYS = 20; /* Should be sufficient */

struct range
{
  int x;
  int y;
};


struct rangekey
{
  string key <41>;
  int x;
  int y;
  int err;
};

struct key_request
{
  string filename < MAXNAMELEN >;
  range ranges <MAXKEYS>;
};

struct key_rtn
{
  rangekey keys <MAXKEYS>;
  int err;
};

program KDS_RPC
{
  version KEYREQVERS
  {
    struct key_rtn KEYREQ (struct key_request) = 1;
  } = 1;
} = 20380;
