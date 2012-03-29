#!/bin/sh -x

rm -f horus-benchmark-kds-1.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-1.data

rm -f horus-benchmark-kds-2.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-2.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-2.data

rm -f horus-benchmark-kds-3.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-3.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-3.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-3.data

rm -f horus-benchmark-kds-4.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-4.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-4.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-4.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-4.data

rm -f horus-benchmark-kds-5.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-5.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-5.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-5.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-5.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-5.data

rm -f horus-benchmark-kds-6.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-6.data

rm -f horus-benchmark-kds-7.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-7.data

rm -f horus-benchmark-kds-8.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-8.data

rm -f horus-benchmark-kds-9.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-9.data

rm -f horus-benchmark-kds-10.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-10.data

rm -f horus-benchmark-kds-11.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-11.data

rm -f horus-benchmark-kds-12.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-12.data


rm -f horus-benchmark-kds-16.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-16.data

rm -f horus-benchmark-kds-32.data
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data &
~/bin/kds_client FILENAME -v -b -l 100M >> horus-benchmark-kds-32.data


rm -f horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-1.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-2.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-3.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-4.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-5.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-6.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-7.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-8.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-9.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-10.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-11.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-12.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-16.data >> horus-benchmark-kds-total.data
awk -f horus-benchmark-kds.awk horus-benchmark-kds-32.data >> horus-benchmark-kds-total.data



