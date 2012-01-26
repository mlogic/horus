#!/bin/sh -x

pics=`ls *.pic`
for i in $pics; do
  name=`echo $i | sed -e 's/\.pic$//'`
  echo "pic -t $name.pic > $name.tex"
  pic -t $name.pic > $name.tex
done

