BEGIN {
  alevel = 0;
  qps = 0;
}

/^alevel: / {
  alevel = $2;
}

/^benchmark: per-thread-simulated: / {
  qps = $7;
  printf "alevel: %d performance: %f qps\n", alevel, qps;
}

END {
}

