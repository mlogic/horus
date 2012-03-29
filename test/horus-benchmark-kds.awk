BEGIN {
  n = 0;
  s = 0;
}

/q\/s/ {
  n++;
  s += $7;
  # print $7;
}

END {
  printf "process %d sum of q/s: %f\n", n, s;
}

