BEGIN {
  nthread = 0;
  qps = 0;
}

/^nthread: / {
  nthread = $2;
}

/^benchmark: per-thread-requested: / {
  qps = $6;
  printf "number-of-client-thread: %d performance: %f qps\n", nthread, qps;
}

END {
}

