BEGIN {
  branch = 0;
  depth = 0;
  qps = 0;
}

/^filename: / {
  split($2,a,"B");
  split(a[2],b,"D");
  branch = b[1]
  depth = b[2];
}

/^benchmark: per-thread-simulated: / {
  qps = $6;
  printf "kht-branch-depth: %d performance: %f qps\n", depth, qps;
}

END {
}

