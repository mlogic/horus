#include "horusfs.h"

/* 
 * Usage :
 * horus_attr_util filename add client_ip start_block end_block
 * horus_attr_util filename set masterkey
 * horus_attr_util filename list*/

int
main (int argc, char *argv[])
{
  struct in_addr addr;
  int start_block, end_block, ret, usage = 0;

  if (argc >= 3)
    {
      if (strcasecmp (argv[2], "list") == 0)
        {
          horus_ea_config_show (argv[1]);
        }
      else if (strcasecmp (argv[2], "add") == 0)
        {
          if (argc != 6)
            {
              usage = 1;
              goto exit;
            }
          inet_aton (argv[3], &addr);
          start_block = atoi (argv[4]);
          end_block = atoi (argv[5]);
          ret =
            horus_ea_config_add_entry (argv[1], -1, addr, start_block,
                                       end_block);
        }
      else if (strcasecmp (argv[2], "set") == 0)
        {
          if (argc != 4)
            {
              usage = 1;
              goto exit;
            }
          ret = horus_ea_config_masterkey (argv[1], -1, argv[3]);
        }
      else
        usage = 1;
    }
  else
    usage = 1;
exit:
  if (usage)
    {
      printf ("Usage :\n");
      printf ("\t%s filename add client_ip start_block end_block\n", argv[0]);
      printf ("\t%s filename set masterkey\n", argv[0]);
      printf ("\t%s filename list\n", argv[0]);
    }
  if (ret != 0)
    printf ("err %s", strerror (errno));
}
