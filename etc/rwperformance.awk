BEGIN {
  length = 0;
  time = 0;
  iops = 0;
  bps = 0;
}

/^time: / {
  length = $3;
  time = $7;
}

/^I\/Ops: / {
  iops = $2;
  bps = $5;
  printf "rwperformance: %d %f bps %f iops \n", length, bps, iops;
}

END {
}

