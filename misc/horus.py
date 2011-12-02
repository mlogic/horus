#!/usr/bin/python

import sys
import hashlib, hmac

def block_key (base_key, file_size, byte_offset, start_level=0, end_level=9):
  '''
  Return the block key to be used for a given byte offset.  This assumes
  a maximum chunk size of 1 GB and a minimum chunk size of 4 KB, with a
  branching factor of 4 between the levels.  This means 9 branches to
  go from 1 GB down to 4 KB.
  '''
  blk_sizes = [1073741824, 268435456, 67108864, 16777216, 4194304,
               1048576, 262144, 65536, 16384, 4096]
  if byte_offset >= file_size:
    raise ValueError
  if not 0 <= start_level <= len (blk_sizes)-1:
    raise ValueError, 'invalid start level %d' % (start_level)
  if not 1 <= end_level <= len (blk_sizes):
    raise ValueError, 'invalid end level %d' % (end_level)
  if start_level >= end_level:
    raise ValueError, 'start level %d must be less than end level %d' % (start_level, end_level)
  cur_offset = byte_offset
  if type (base_key) != str:
    cur_key = '%016x' % base_key
  else:
    cur_key = base_key
  for i in range (start_level, end_level+1):
    if i > 0:
      max_blks = blk_sizes[i-1] / blk_sizes[i]
    else:
      max_blks = 1000000000
    blk_num = (byte_offset / blk_sizes[i])
    cur_offset = byte_offset % blk_sizes[i]
    h = hmac.new (cur_key, '%016x' % ((i << 24) | blk_num), hashlib.sha1)
    cur_key = h.hexdigest ()
    print "K" + str(i) + "," + str(blk_num) + " = " + cur_key,
    print "[" + '%016x' % ((i << 24) | blk_num) + "]"

  return cur_key


argv = sys.argv
argc = len(argv)

if (argc != 3):
  print 'Usage: %% python %s offset masterkey' % argv[0]
  quit()

print 'masterkey=\'%s\' file_size=3M offset=%s' % (argv[2], argv[1])
print block_key (argv[2], 3000000, int(argv[1]), 0, 9)


