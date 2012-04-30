BEGIN {
  len = 0;
  time = 0;
  iops = 0;
  bps = 0;
}

/^time: / {
  len = $3;
  time = $7;
}

/^I\/Ops: / {
  iops = $2;
  bps = $5;
  printf "rwperformance: %d %f bps %f iops \n", len, bps, iops;
}

END {
}

