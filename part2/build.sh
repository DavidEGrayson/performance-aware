#!/usr/bin/bash -ue

gcc -g -Og -Wall repeat_write_bytes.c -o repeat_write_bytes

# gcc -g -Wall haversine_sum.c -o haversine_sum
#gcc -g -Wall haversine_sum.c -DPROFILE -o haversine_sum_p

#echo No profiler
#./haversine_sum
#echo
#./haversine_sum_p
