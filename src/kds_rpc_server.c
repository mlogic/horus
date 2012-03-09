#include <horus.h>
#include <openssl.h>
#include <horus_attr.h>
#include <horus_key.h>

#include <rpc/rpc.h>
#include "kds_rpc.h"
#include "kds_rpc_server.h"

int ea_cache_add (uint64_t inode_num, struct horus_ea_config *config_ptr)
{

  EA_CACHE_ENTRY *temp_entry = NULL,*temp;
  int ret = 0,i;

  temp_entry = (EA_CACHE_ENTRY *) malloc(sizeof (EA_CACHE_ENTRY));

  if (temp_entry)
    {
      temp_entry->inode_num = inode_num;
      memcpy(&temp_entry->config, config_ptr, HORUS_EA_SIZE);
      HASH_ADD (hh, ea_cache, inode_num, 8, temp_entry);

      if (HASH_COUNT (ea_cache) >= MAX_EA_CACHE)
	{
	  for (i=0 ; i < MAX_EA_CACHE/10; i++)
	    {
	      HASH_ITER(hh,ea_cache, temp_entry,temp)
		{
		  HASH_DELETE(hh, ea_cache, temp_entry);
		  free(temp_entry);
		  break;
		}
	    }
	}
      ret = 0;
    }
  else
    {
      fprintf(stderr, "Malloc failure\n");
      ret = -1;
    }
  return ret;
}


int ea_cache_find (uint64_t inode_num, EA_CACHE_ENTRY **temp_entry)
{
  int ret = 0;
  HASH_FIND (hh, ea_cache, &inode_num, 8, (*temp_entry));

  if (*temp_entry)
    {
      HASH_DELETE(hh, ea_cache, (*temp_entry));
      HASH_ADD (hh, ea_cache, inode_num, 8, *temp_entry);
      ret = 0;
    }
  else
    {
      ret = -1;
    }

  return ret;
}
  
struct key_rtn *
keyreq_1_svc (struct key_request *input, struct svc_req *req)
{

  int fd = -1, i;
  uint32_t start_block, end_block, x, y, range_size, blk, start, end;
  int ret, key_len, parent_len,ranges_len;
  char key[HORUS_KEY_LEN], range[30], parent[30];
  char master[30];
  char *filename;
  struct sockaddr_in *client_addr;
  EA_CACHE_ENTRY *cache_ptr;
  struct stat statinfo;
  struct horus_ea_config config, *config_ptr;
  int *kht_branching_factor = NULL;
  static struct key_rtn key_out;

  xdr_free (xdr_key_rtn, &key_out);

  key_out.err = -1;

  filename = input->filename;
  ranges_len = input->ranges.ranges_len;
  client_addr = svc_getcaller (req->rq_xprt);


  if(debug)
    fprintf (stderr, "Got request for %s num of keys %d\n", filename, ranges_len);

  key_out.keys.keys_len=0;
  key_out.err = 0;
  fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      if(debug)
        fprintf (stderr, "Unable to open %s!\n", filename);
      key_out.err = EIO;
      goto exit;
    }
  else
    {
      fstat (fd, &statinfo);  /* Check for error code */
      ret = ea_cache_find(statinfo.st_ino, &cache_ptr);
      if (ret != 0)  /* Not found in cache */
	{
	  ret = horus_get_fattr(fd, &config);
	  if (ret == 0)
	    {
          
	      config_ptr = &config;
	      ret = ea_cache_add(statinfo.st_ino, &config);
	      /* TODO : Handled ret code */
        

	    }
	  /* TODO: Handle ret code */
	}
      else
	{
	  config_ptr = &cache_ptr->config;
	  if(debug)
	    fprintf(stderr, "Found in cache!\n");
	}

      ret = horus_get_fattr_config_client(config_ptr,
                                          &(client_addr->sin_addr),
                                          &start_block, &end_block);
      if (ret != 0)
	{
	  if(debug)
	    {
	      fprintf (stderr, "Range for %s not configured for client\n",
		       filename);
	    }
	  key_out.err = EINVAL;
	  goto exit;
	}

      ret = horus_get_fattr_masterkey_config (config_ptr, master, 30);
      if (ret < 0)
        {
          if(debug)
            fprintf (stderr, "Unable to read master-key ret = %d!\n", ret);
          key_out.err = EINVAL;
          goto exit;
        }
      master[ret + 1] = '\0';   /* ret actually has length of master-key */
      kht_branching_factor = malloc ((int)(*config_ptr->kht_depth) - 1);
      if (NULL == kht_branching_factor)
	abort (); // TODO
      for (i = 0; i < config_ptr->kht_depth-1; ++i)
	kht_branching_factor[i] = config_ptr->branching_factor;
      horus_set_kht (fd,
		     config_ptr->kht_depth,
		     MIN_CHUNK_SIZE,
		     kht_branching_factor);
      horus_add_key (fd, master, strnlen (master, HORUS_KEY_LEN), 0, 0);

      key_out.keys.keys_len = ranges_len;
      key_out.keys.keys_val = calloc(sizeof(struct rangekey), ranges_len);

      for (i = 0; i < ranges_len; i++)
	{
	  /* Convert x,y into start-block  and end-block  number */
	  x = input->ranges.ranges_val[i].x;
	  y = input->ranges.ranges_val[i].y;

 
	  start = (y * horus_get_block_size (fd, x) / MIN_CHUNK_SIZE);
	  end = ((y + 1) * horus_get_block_size (fd, x) / MIN_CHUNK_SIZE);
	  if ((start >= start_block) && (end <= end_block))
	    {
	      snprintf (parent, sizeof (parent), "%s", master);
	      parent_len = strlen (master);
	      ret = horus_get_key (fd, key, x, y);
	      if (ret == 0)
		{
		  key_out.keys.keys_val[i].key = strdup (print_key (key, key_len));
		  if (debug)
		    fprintf(stderr, "%d %d %s\n",  x, y, key_out.keys.keys_val[i].key);
		  key_out.keys.keys_val[i].err = 0;
		}
	      else
		{
		  if (debug)
		    fprintf (stderr, "Error: horus_key_by_master error %d\n", ret);
		  key_out.keys.keys_val[i].key =  strdup ("");
		  key_out.keys.keys_val[i].err = ret;
		}
	    }
	  else
	    {
	      if (debug)
		fprintf (stderr, "Access denied!\n");
	      key_out.keys.keys_val[i].err  = EACCES;
	      key_out.keys.keys_val[i].key =  strdup ("");
	    }
	}
    }
 exit:
  if (fd > 0)
    close (fd);
  if (kht_branching_factor)
    free (kht_branching_factor);
  return &key_out;
}
